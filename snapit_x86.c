/*
 * This file is part of 'snapit' program.
 *
 * Copyright (c) 2012 Alexandr Kozlinskiy
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:

 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>

#define STRICT 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <shellapi.h>

UINT WMU_SNAPIT_UNINSTALL;

int hook_install();
int hook_uninstall();

const char* g_title = "SnapIt";
HICON g_icon;

const UINT WMU_TRAYICON = WM_USER + 1;
NOTIFYICONDATA g_notifyIconData;
const UINT ID_MENU_INSTALL = 0xFF00 + 1;
const UINT ID_MENU_UNINSTALL = 0xFF00 + 2;
const UINT ID_MENU_EXIT = 0xFF00 + 3;
HMENU g_menu;

LRESULT CALLBACK fproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  switch(msg)
  {
  case WMU_TRAYICON:
    printf("WMU_TRAYICON\n");
    switch(lp)
    {
    case WM_LBUTTONUP:
//      ShowWindow(hwnd, SW_SHOW);
//      SetForegroundWindow(hwnd);
      break;
    case WM_RBUTTONUP:
      POINT point;
      GetCursorPos(&point);

      SetForegroundWindow(hwnd);
      switch(TrackPopupMenu(g_menu, TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, 0, hwnd, NULL))
      {
      case ID_MENU_INSTALL:
        EnableMenuItem(g_menu, ID_MENU_INSTALL, MF_GRAYED);
        hook_install();
        EnableMenuItem(g_menu, ID_MENU_UNINSTALL, MF_ENABLED);
        break;
      case ID_MENU_UNINSTALL:
        EnableMenuItem(g_menu, ID_MENU_UNINSTALL, MF_GRAYED);
        SendMessageTimeout(HWND_BROADCAST, WMU_SNAPIT_UNINSTALL, 0, 0, SMTO_NORMAL, 250, NULL);
        hook_uninstall();
        EnableMenuItem(g_menu, ID_MENU_INSTALL, MF_ENABLED);
        break;
      case ID_MENU_EXIT:
        PostMessage(hwnd, WM_CLOSE, 0, 0);
        break;
      }
      break;
    }
    return 0; break;
  case WM_CREATE:
    break;
  case WM_SYSCOMMAND:
    printf("WM_SYSCOMMAND\n");
    switch(wp & 0xFFF0)
    {
    case SC_MINIMIZE:
      printf("SC_MINIMIZE\n");
      ShowWindow(hwnd, SW_HIDE);
      return 0; break;
    }
    break;
  case WM_CLOSE:
    DestroyWindow(hwnd);
    return 0; break;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0; break;
  }

  return DefWindowProc(hwnd, msg, wp, lp);
}

int CALLBACK WinMain(HINSTANCE hinst, HINSTANCE hprev, LPSTR cmd, int show)
{
//  AllocConsole();
//  AttachConsole(GetCurrentProcessId());
#pragma warning(push)
#pragma warning( disable : 4996 )
//  freopen("CON", "w", stdout);
#pragma warning(pop)

  WMU_SNAPIT_UNINSTALL =
  #if defined(WIN64)
    RegisterWindowMessage("WMU_SNAPIT_UNINSTALL_{494e0de4-493b-4d30-9eb5-e7de12b247c0}");
  #else
    RegisterWindowMessage("WMU_SNAPIT_UNINSTALL_{faa9d599-79d1-4112-ac68-1263a84c1d24}");
  #endif

  g_icon = (HICON)LoadImage(NULL, "snapit.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

  WNDCLASSEX wc;
  ZeroMemory(&wc, sizeof(wc));
  wc.cbSize        = sizeof(wc);
  wc.style         = CS_VREDRAW | CS_HREDRAW;
  wc.lpfnWndProc   = fproc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hIcon         = g_icon;
  wc.hCursor       = LoadCursor(0, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName  = 0;
  wc.lpszClassName = g_title;
  wc.hInstance     = hinst;
  wc.hIconSm       = g_icon;

  if(!RegisterClassEx(&wc)) return 0;

  HWND hwnd = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    g_title, g_title,
    WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    CW_USEDEFAULT, CW_USEDEFAULT, 160, 120,
    0, 0, hinst, 0);

  if(hwnd == 0) return 0;

  memset(&g_notifyIconData, 0, sizeof(g_notifyIconData));
  g_notifyIconData.hWnd = hwnd;
  g_notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE;
  g_notifyIconData.uCallbackMessage = WMU_TRAYICON;
  g_notifyIconData.hIcon = g_icon;

  g_menu = CreatePopupMenu();
  AppendMenu(g_menu, MF_STRING, ID_MENU_INSTALL, "Install");
  AppendMenu(g_menu, MF_STRING | MF_GRAYED, ID_MENU_UNINSTALL, "Uninstall");
  AppendMenu(g_menu, MF_STRING, ID_MENU_EXIT, "Exit");

  Shell_NotifyIcon(NIM_ADD, &g_notifyIconData);

  MSG msg;
  while(GetMessage(&msg, 0, 0, 0) > 0)
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  Shell_NotifyIcon(NIM_DELETE, &g_notifyIconData);

  hook_uninstall();
  SendMessageTimeout(HWND_BROADCAST, WMU_SNAPIT_UNINSTALL, 0, 0, SMTO_NORMAL, 250, NULL);

  return 0;
}

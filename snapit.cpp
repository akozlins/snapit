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

#if defined(WIN64)
const char* g_mutex_name = "MUTEX_SNAPIT_x64_{494e0de4-493b-4d30-9eb5-e7de12b247c0}";
const char* g_message_name = "WMU_SNAPIT_UNINSTALL_x64_{494e0de4-493b-4d30-9eb5-e7de12b247c0}";
const char* g_class_name = "SnapIt_x64_{494e0de4-493b-4d30-9eb5-e7de12b247c0}";
const char* g_title = "SnapItX64";
const char* g_icon_name = "snapit_x64.ico";
#else
const char* g_mutex_name = "MUTEX_SNAPIT_x32_{494e0de4-493b-4d30-9eb5-e7de12b247c0}";
const char* g_message_name = "WMU_SNAPIT_UNINSTALL_x32_{494e0de4-493b-4d30-9eb5-e7de12b247c0}";
const char* g_class_name = "SnapIt_x32_{494e0de4-493b-4d30-9eb5-e7de12b247c0}";
const char* g_title = "SnapItX32";
const char* g_icon_name = "snapit_x32.ico";
#endif

NOTIFYICONDATA g_idata;
HICON g_icon = 0;

const UINT WMU_TRAYICON = WM_USER + 1;
const UINT ID_MENU_INSTALL   = 0xFF00 + 1;
const UINT ID_MENU_UNINSTALL = 0xFF00 + 2;
const UINT ID_MENU_HIDE      = 0xFF00 + 3;
const UINT ID_MENU_EXIT      = 0xFF00 + 4;
HMENU g_menu = 0;

HWND hwndEdit = 0;

void hook_install_()
{
  EnableMenuItem(g_menu, ID_MENU_INSTALL, MF_GRAYED);
  hook_install();
  EnableMenuItem(g_menu, ID_MENU_UNINSTALL, MF_ENABLED);
}

void hook_uninstall_()
{
  EnableMenuItem(g_menu, ID_MENU_UNINSTALL, MF_GRAYED);
  SendMessageTimeout(HWND_BROADCAST, WMU_SNAPIT_UNINSTALL, 0, 0, SMTO_NORMAL, 500, NULL);
  hook_uninstall();
  EnableMenuItem(g_menu, ID_MENU_INSTALL, MF_ENABLED);
}

LRESULT CALLBACK fproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
  if(msg == WMU_TRAYICON)
  {
    switch(lp)
    {
    case WM_LBUTTONUP:
      SetForegroundWindow(hwnd);
      ShowWindow(hwnd, SW_SHOW);
      break;
    case WM_RBUTTONUP:
      SetForegroundWindow(hwnd);

      POINT point;
      GetCursorPos(&point);

      switch(TrackPopupMenu(g_menu, TPM_RETURNCMD | TPM_NONOTIFY, point.x, point.y, 0, hwnd, NULL))
      {
      case ID_MENU_INSTALL:
        hook_install_();
        break;
      case ID_MENU_UNINSTALL:
        hook_uninstall_();
        break;
      case ID_MENU_HIDE:
        Shell_NotifyIcon(NIM_DELETE, &g_idata);
        break;
      case ID_MENU_EXIT:
        PostMessage(hwnd, WM_CLOSE, 0, 0);
        break;
      }
      break;
    }
    return 0;
  }

  switch(msg)
  {
  case WM_CREATE:
    hwndEdit = CreateWindow(
      "edit", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
      0, 0, 0, 0,
      hwnd, NULL, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    break;
  case WM_SIZE:
    MoveWindow(hwndEdit, 0, 0, LOWORD(lp), HIWORD(lp), TRUE);
    break;
  case WM_SYSCOMMAND:
    switch(wp & 0xFFF0)
    {
    case SC_MINIMIZE:
      Shell_NotifyIcon(NIM_ADD, &g_idata);
      ShowWindow(hwnd, SW_HIDE);
      return 0; break;
    }
    break;
  case WM_CLOSE:
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
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

  HANDLE mutex = CreateMutex(NULL, TRUE, g_mutex_name);
  if(!mutex) return 0;

  if(GetLastError() == ERROR_ALREADY_EXISTS)
  {
    HWND hwnd = FindWindow(g_class_name, g_title);
    if(hwnd)
    {
      SetForegroundWindow(hwnd);
      ShowWindow(hwnd, SW_SHOW);
    }
    ReleaseMutex(mutex);
    CloseHandle(mutex);
    return 0;
  }

  WMU_SNAPIT_UNINSTALL = RegisterWindowMessage(g_message_name);

  g_icon = (HICON)LoadImage(NULL, g_icon_name, IMAGE_ICON, 0, 0, LR_LOADFROMFILE);

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
  wc.lpszClassName = g_class_name;
  wc.hInstance     = hinst;
  wc.hIconSm       = g_icon;

  RegisterClassEx(&wc);

  HWND hwnd = CreateWindowEx(
    WS_EX_CLIENTEDGE,
    g_class_name, g_title,
    WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
    CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
    0, 0, hinst, 0);

  ReleaseMutex(mutex);

  if(hwnd == 0) return 0;

  memset(&g_idata, 0, sizeof(g_idata));
  g_idata.cbSize = sizeof(g_idata);
  g_idata.hWnd = hwnd;
  g_idata.uFlags = NIF_MESSAGE | NIF_ICON;
  g_idata.uCallbackMessage = WMU_TRAYICON;
  g_idata.hIcon = g_icon;

  g_menu = CreatePopupMenu();
  AppendMenu(g_menu, MF_STRING | MF_GRAYED, ID_MENU_INSTALL, "Install");
  AppendMenu(g_menu, MF_STRING | MF_GRAYED, ID_MENU_UNINSTALL, "Uninstall");
  AppendMenu(g_menu, MF_STRING, ID_MENU_HIDE, "Hide");
  AppendMenu(g_menu, MF_STRING, ID_MENU_EXIT, "Exit");

  Shell_NotifyIcon(NIM_ADD, &g_idata);
  hook_install_();

  MSG msg;
  while(GetMessage(&msg, 0, 0, 0) > 0)
  {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  hook_uninstall_();
  Shell_NotifyIcon(NIM_DELETE, &g_idata);

  CloseHandle(mutex);
  return 0;
}

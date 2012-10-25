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
#include <commctrl.h>
#include <psapi.h>

#define DLL_EXPORT __declspec(dllexport)

#define DX 16
#define DY 16

#pragma data_seg(".sdata")
HHOOK g_hhook = 0;
#pragma data_seg()
#pragma comment(linker, "/section:.sdata,rws")

HINSTANCE g_hinst = 0;

#define _log_ flog

void flog(const char* fmt, ...)
{
#pragma warning(push)
#pragma warning( disable : 4996 )
  FILE* file = fopen("d:/out.txt", "a+");
#pragma warning(pop)
  if(!file) return;

  va_list list;
  va_start(list, fmt);
  vfprintf(file, fmt, list);
  va_end(list);

  fclose(file);
} // flog

typedef struct {
  HWND hwnd;
  RECT rect_list[64];
  int rect_list_n;
} STATE;

BOOL CALLBACK fenum(HWND hwnd, LPARAM lp)
{
  STATE* state = (STATE*)lp;

  if(state->rect_list_n == 64) return FALSE;

  if(hwnd == state->hwnd ||
    !IsWindowVisible(hwnd) ||
    IsIconic(hwnd) ||
    (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_MDICHILD)) return TRUE;

  RECT rect;
  if(!GetWindowRect(hwnd, &rect)) return TRUE;
  state->rect_list[state->rect_list_n++] = rect;

/*  HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
  if(GetWindowRgn(hwnd, hrgn) != ERROR)
  {
    GetRgnBox()
    CombineRgn()
  }
  DeleteObject(hrgn);*/

//  if(lp == 0) EnumChildWindows(hwnd, fenum, 1)

  return TRUE;
} // fenum

void fposchanging(STATE* state, PWINDOWPOS pos)
{
  if(pos->flags & SWP_NOMOVE && pos->flags & SWP_NOSIZE) return;
  _log_("  x = %d, y = %d, w = %d, h = %d\n", pos->x, pos->y, pos->cx, pos->cy);

  RECT rect; // current window position
  if(!GetWindowRect(state->hwnd, &rect)) return;

  // next windown position
  int l = pos->x, r = pos->x + pos->cx, t = pos->y, b = pos->y + pos->cy;

  // distance to closest window border
  int dl = INT_MAX, dr = INT_MAX, dt = INT_MAX, db = INT_MAX;
  for(int i = 0; i < state->rect_list_n; i++)
  {
    RECT* rect_ = state->rect_list + i;
    int l_ = rect_->left, r_ = rect_->right, t_ = rect_->top, b_ = rect_->bottom;

    // don't snap to left/right border cont.
    if(t < b_ + DY && b > t_ - DY)
    {
      int dl_ = l_ - l; if(abs(dl_) < abs(dl)) dl = dl_;
          dl_ = r_ - l; if(abs(dl_) < abs(dl)) dl = dl_;
      int dr_ = l_ - r; if(abs(dr_) < abs(dr)) dr = dr_;
          dr_ = r_ - r; if(abs(dr_) < abs(dr)) dr = dr_;
    }

    // don't snap to top/bottom border cont.
    if(l < r_ + DX && r > l_ - DX)
    {
      int dt_ = t_ - t; if(abs(dt_) < abs(dt)) dt = dt_;
          dt_ = b_ - t; if(abs(dt_) < abs(dt)) dt = dt_;
      int db_ = t_ - b; if(abs(db_) < abs(db)) db = db_;
          db_ = b_ - b; if(abs(db_) < abs(db)) db = db_;
    }
  }
  _log_("  dl = %d, dr = %d, dt = %d, db = %d\n", dl, dr, dt, db);

  if(!(pos->flags & SWP_NOMOVE) &&
    pos->cx == (rect.right - rect.left) && pos->cy == (rect.bottom - rect.top) &&
    (pos->x != rect.left || pos->y != rect.top))
  {
    int dx = dl;
    if(abs(dr) < abs(dx)) dx = dr;
    if(abs(dx) < DX) pos->x = l + dx;

    int dy = dt;
    if(abs(db) < abs(dy)) dy = db;
    if(abs(dy) < DY) pos->y = t + dy;
  }
  else if(!(pos->flags & SWP_NOSIZE))
  {
    if(pos->cx != (rect.right - rect.left))
    {
           if(pos->x == rect.left            && abs(dr) < DX && r - l + dr > 0) { pos->cx = r - l + dr; }
      else if(pos->x == rect.right - pos->cx && abs(dl) < DX && r - l - dl > 0) { pos->cx = r - l - dl; pos->x = l + dl; }
    }
    if(pos->cy != (rect.bottom - rect.top))
    {
           if(pos->y == rect.top              && abs(db) < DY && b - t + db > 0) { pos->cy = b - t + db; }
      else if(pos->y == rect.bottom - pos->cy && abs(dt) < DY && b - t - dt > 0) { pos->cy = b - t - dt; pos->y = t + dt; }
    }
  }
} // fproc_

LRESULT CALLBACK fproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR state)
{
  switch(msg)
  {
  case WM_ENTERSIZEMOVE:
    _log_("fproc | WM_ENTERSIZEMOVE: thread = %d, hwnd = %08X\n", GetCurrentThreadId(), hwnd);
    EnumWindows(fenum, state);
    break;
  case WM_NCDESTROY:
  case WM_EXITSIZEMOVE:
    _log_("fproc | RemoveWindowSubclass: thread = %d, hwnd = %08X\n", GetCurrentThreadId(), hwnd);
    if(RemoveWindowSubclass(hwnd, fproc, 0)) free((void*)state);
    break;
  case WM_WINDOWPOSCHANGING:
    _log_("fproc | WM_WINDOWPOSCHANGING: thread = %d, hwnd = %08X\n", GetCurrentThreadId(), hwnd);
    fposchanging((STATE*)state, (PWINDOWPOS)lp);
    break;
  }

  return DefSubclassProc(hwnd, msg, wp, lp);
} // fproc

DLL_EXPORT LRESULT CALLBACK fhook(int code, WPARAM wp, LPARAM lp)
{
  if(code < 0 || wp != 0 || lp == 0) return CallNextHookEx(g_hhook, code, wp, lp);

  PCWPSTRUCT cwp = (PCWPSTRUCT)lp;
  HWND hwnd = cwp->hwnd;

  if(cwp->message == WM_ENTERSIZEMOVE)
  {
    if(IsZoomed(hwnd) || (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_MDICHILD)) return CallNextHookEx(g_hhook, code, wp, lp);

    _log_("fhook | SetWindowSubclass: thread = %d, hwnd = %08X\n", GetCurrentThreadId(), hwnd);
    STATE* state = (STATE*)malloc(sizeof(STATE));
    if(state)
    {
       memset(state, 0, sizeof(STATE));
       state->hwnd = hwnd;
       if(!SetWindowSubclass(hwnd, fproc, 0, (DWORD_PTR)state)) free(state);
    }
  }

  return CallNextHookEx(g_hhook, code, wp, lp);
} // fhook

DLL_EXPORT int hook_install()
{
  _log_("set hook ...\n");

  if(!g_hhook) g_hhook = SetWindowsHookEx(WH_CALLWNDPROC, fhook, g_hinst, 0);
  _log_("  hhook = %08X\n", g_hhook);

  return (g_hhook ? 0 : -1);
} // install

DLL_EXPORT int hook_uninstall()
{
  if(!g_hhook) return 0;

  _log_("remove hook ...\n");
  if(UnhookWindowsHookEx(g_hhook))
  {
    g_hhook = 0;
    _log_("  OK\n");
    return 0;
  }
  else
  {
    _log_("  FAIL\n");
    return -1;
  }
} // uninstall

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
  char buf[256];
  HANDLE hproc = GetCurrentProcess();
  GetModuleFileNameEx(hproc, 0, buf, 256);
  CloseHandle(hproc);
  _log_("DllMain: hinst = %08X, file = %s\n", hinst, buf);

  if(reason == DLL_PROCESS_ATTACH)
  {
    _log_("  DLL_PROCESS_ATTACH\n");

    g_hinst = hinst;
    DisableThreadLibraryCalls(hinst);
  }

  if(reason == DLL_PROCESS_DETACH)
  {
    _log_("  DLL_PROCESS_DETACH\n");
  }

  return TRUE;
} // DllMain

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
#include <share.h>

#define STRICT 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <commctrl.h>
#include <psapi.h>

#include "snapit.h"

UINT WMU_SNAPIT_UNINSTALL;

#define DLL_EXPORT __declspec(dllexport)

#define DX 16
#define DY 16

#pragma data_seg(".sdata")
HHOOK g_hhook = 0;
volatile long g_lock_log = 0;
#pragma data_seg()
#pragma comment(linker, "/section:.sdata,rws")

HINSTANCE g_hinst = 0;
char g_file_proc[256];
char g_file_log[256];

#define _log_ flog

void flog(const char* fmt, ...)
{
  int lock_i = 0;
  while(InterlockedExchange(&g_lock_log, 1) == 1) lock_i++;
  FILE* file = _fsopen(g_file_log, "a", _SH_DENYNO);
  if(file)
  {
    va_list list;
    va_start(list, fmt);
    vfprintf(file, fmt, list);
    va_end(list);

    if(lock_i > 0) fprintf(file, " lock_i = %d\n", lock_i);
    else fprintf(file, "\n");

    fclose(file);
  }
  InterlockedExchange(&g_lock_log, 0);

  HWND hwnd = FindWindow(g_class_name, g_title);
  if(hwnd)
  {
    char buffer[64];

    va_list list;
    va_start(list, fmt);
    int n = vsnprintf_s(buffer, sizeof(buffer) - 2, _TRUNCATE, fmt, list);
    va_end(list);

    if(n < 0) n = sizeof(buffer) - 3;

    buffer[n++] = 0x0D;
    buffer[n++] = 0x0A;
    buffer[n++] = 0x00;
  
    COPYDATASTRUCT data = { COPYDATA_LOG_ID, sizeof(buffer), buffer };
    if(!SendMessageTimeout(hwnd, WM_COPYDATA, 0, (LPARAM)&data, SMTO_NORMAL, 10, 0))
    {
      DWORD e = GetLastError();
    }
    
  }
} // flog

typedef struct {
  HWND hwnd;
  RECT rect_list[32];
  int rect_list_n;
} STATE;

BOOL WINAPI IsChild(HWND hwnd)
{
  return (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_MDICHILD) ? TRUE : FALSE;
}

void fenum(STATE* state)
{
  HWND hwnd = state->hwnd;

  HWND hwndDesktop = GetDesktopWindow();
  HWND hwndStart = FindWindowEx(hwndDesktop, NULL, "Button", "Start");

  RECT& rectDesktop = state->rect_list[state->rect_list_n];
  if(!GetWindowRect(hwndDesktop, &rectDesktop)) return;
  state->rect_list_n++;

  HRGN hrgnDesktop = CreateRectRgn(rectDesktop.left, rectDesktop.top, rectDesktop.right, rectDesktop.bottom);
  HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
  HRGN hrgn_ = CreateRectRgn(0, 0, 0, 0);
  for(HWND hwnd_ = GetTopWindow(NULL); hwnd_ != NULL; hwnd_ = GetWindow(hwnd_, GW_HWNDNEXT))
  {
    if(hwnd_ == hwnd || hwnd_ == hwndStart || hwnd_ == hwndDesktop ||
      !IsWindowVisible(hwnd_) || IsIconic(hwnd_) || IsChild(hwnd)) continue;

    RECT& rect = state->rect_list[state->rect_list_n];
    if(!GetWindowRect(hwnd_, &rect)) continue;

    if(!SetRectRgn(hrgn_, rect.left, rect.top, rect.right, rect.bottom)) continue;
    if(hrgnDesktop)
    {
      if(CombineRgn(hrgn_, hrgn_, hrgnDesktop, RGN_AND) == NULLREGION) continue;
      if(GetRgnBox(hrgn_, &rect) == NULLREGION) continue;
    }
    if(CombineRgn(hrgn_, hrgn_, hrgn, RGN_DIFF) == NULLREGION) continue;
    if(CombineRgn(hrgn, hrgn, hrgn_, RGN_OR) == ERROR) continue;

    if(state->rect_list_n++ == sizeof(state->rect_list) / sizeof(RECT)) break;
  }
  DeleteObject(hrgn_);
  DeleteObject(hrgn);
  DeleteObject(hrgnDesktop);
} // fenum

void fposchanging(STATE* state, PWINDOWPOS pos)
{
  if(pos->flags & SWP_NOMOVE && pos->flags & SWP_NOSIZE) return;

  // next windown position
  const long &l = pos->x, &w = pos->cx, &t = pos->y, &h = pos->cy;
  const long r = l + w, b = t + h;

  // distance to closest window border
  long dl = LONG_MAX, dr = LONG_MAX, dt = LONG_MAX, db = LONG_MAX;
  for(int i = 0; i < state->rect_list_n; i++)
  {
    RECT& rect_ = state->rect_list[i];
    const long &l_ = rect_.left, &r_ = rect_.right, &t_ = rect_.top, &b_ = rect_.bottom;

    // don't snap to left/right border cont.
    if(t < b_ + DY && b > t_ - DY)
    {
      long dl_ = l_ - l; if(abs(dl_) < abs(dl)) dl = dl_;
           dl_ = r_ - l; if(abs(dl_) < abs(dl)) dl = dl_;
      long dr_ = l_ - r; if(abs(dr_) < abs(dr)) dr = dr_;
           dr_ = r_ - r; if(abs(dr_) < abs(dr)) dr = dr_;
    }

    // don't snap to top/bottom border cont.
    if(l < r_ + DX && r > l_ - DX)
    {
      long dt_ = t_ - t; if(abs(dt_) < abs(dt)) dt = dt_;
           dt_ = b_ - t; if(abs(dt_) < abs(dt)) dt = dt_;
      long db_ = t_ - b; if(abs(db_) < abs(db)) db = db_;
           db_ = b_ - b; if(abs(db_) < abs(db)) db = db_;
    }
  }

  RECT rect; // prev window position
  if(!GetWindowRect(state->hwnd, &rect)) return;
  const long &rl = rect.left, &rr = rect.right, &rt = rect.top, &rb = rect.bottom;
  const long rw = rr - rl, rh = rb - rt;

  if(!(pos->flags & SWP_NOMOVE) && w == rw && h == rh && (l != rl || t != rt))
  {
    long dx = dl;
    if(abs(dr) < abs(dx)) dx = dr;
    if(abs(dx) < DX) pos->x = l + dx;

    long dy = dt;
    if(abs(db) < abs(dy)) dy = db;
    if(abs(dy) < DY) pos->y = t + dy;
  }
  else if(!(pos->flags & SWP_NOSIZE))
  {
    if(pos->cx != rw)
    {
           if(l == rl && abs(dr) < DX && r - l + dr > 0) { pos->cx = r - l + dr; }
      else if(r == rr && abs(dl) < DX && r - l - dl > 0) { pos->cx = r - l - dl; pos->x = l + dl; }
    }
    if(pos->cy != rh)
    {
           if(t == rt && abs(db) < DY && b - t + db > 0) { pos->cy = b - t + db; }
      else if(b == rb && abs(dt) < DY && b - t - dt > 0) { pos->cy = b - t - dt; pos->y = t + dt; }
    }
  }
} // fproc_

LRESULT CALLBACK fproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR data)
{
  if(!data) return DefSubclassProc(hwnd, msg, wp, lp);

  STATE* state = (STATE*)data;
  switch(msg)
  {
  case WM_WINDOWPOSCHANGING:
    _log_("fproc | WM_WINDOWPOSCHANGING: thread = %d, hwnd = %08X", GetCurrentThreadId(), hwnd);
    if(!state->hwnd)
    {
      state->hwnd = hwnd;
      fenum(state);
    }
    if(state->rect_list_n > 0) fposchanging(state, (PWINDOWPOS)lp);
    break;
  }

  return DefSubclassProc(hwnd, msg, wp, lp);
} // fproc

void subclass_install(HWND hwnd)
{
  DWORD_PTR state = 0;
  if(GetWindowSubclass(hwnd, fproc, 0, &state) && state) return;

  if(state = (DWORD_PTR)malloc(sizeof(STATE)))
  {
     memset((void*)state, 0, sizeof(STATE));
    _log_("subclass_install: thread = %d, hwnd = %08X", GetCurrentThreadId(), hwnd);
     if(!SetWindowSubclass(hwnd, fproc, 0, state)) free((void*)state);
  }
}

void subclass_uninstall(HWND hwnd)
{
  DWORD_PTR state = 0;
  if(!GetWindowSubclass(hwnd, fproc, 0, &state) || !state) return;

  _log_("subclass_uninstall: thread = %d, hwnd = %08X", GetCurrentThreadId(), hwnd);
  if(RemoveWindowSubclass(hwnd, fproc, 0)) free((void*)state);
}

DLL_EXPORT LRESULT CALLBACK fhook(int code, WPARAM wp, LPARAM lp)
{
  if(code < 0 || lp == 0) return CallNextHookEx(0, code, wp, lp);

  PCWPSTRUCT cwp = (PCWPSTRUCT)lp;
  HWND hwnd = cwp->hwnd;

  switch(cwp->message)
  {
//  case WM_WINDOWPOSCHANGING:
  case WM_ENTERSIZEMOVE:
    if(IsZoomed(hwnd) || IsChild(hwnd)) break;
    subclass_install(hwnd);
    break;
  case WM_NCDESTROY:
  case WM_EXITSIZEMOVE:
    subclass_uninstall(hwnd);
    break;
  }

  if(cwp->message == WMU_SNAPIT_UNINSTALL)
  {
    _log_("fhook | WMU_SNAPIT_UNINSTALL: thread = %d, hwnd = %08X", GetCurrentThreadId(), hwnd);
    subclass_uninstall(hwnd);
  }

  return CallNextHookEx(0, code, wp, lp);
} // fhook

DLL_EXPORT int hook_install()
{
  _log_("set hook ...");

  if(!g_hhook) g_hhook = SetWindowsHookEx(WH_CALLWNDPROC, fhook, g_hinst, 0);
  _log_("  hhook = %08X", g_hhook);

  return (g_hhook ? 0 : -1);
} // install

DLL_EXPORT int hook_uninstall()
{
  if(!g_hhook) return 0;

  _log_("remove hook ...");
  if(UnhookWindowsHookEx(g_hhook))
  {
    g_hhook = 0;
    _log_("  OK");
    return 0;
  }
  else
  {
    _log_("  FAIL");
    return -1;
  }
} // uninstall

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
  if(reason == DLL_PROCESS_ATTACH)
  {
    HANDLE hproc = GetCurrentProcess();
    GetModuleFileNameEx(hproc, 0, g_file_proc, sizeof(g_file_proc));
    CloseHandle(hproc);

    int n = GetModuleFileName(hinst, g_file_log, sizeof(g_file_log));
    while(g_file_log[n] != '\\') n--;
    #if defined(WIN64)
      strcpy_s(g_file_log + n, sizeof(g_file_log) - n, "\\snapit_x64.log");
    #else
      strcpy_s(g_file_log + n, sizeof(g_file_log) - n, "\\snapit_x32.log");
    #endif

    _log_("attach -> %s", g_file_proc);

    WMU_SNAPIT_UNINSTALL = RegisterWindowMessage(g_message_name);

    g_hinst = hinst;
    DisableThreadLibraryCalls(hinst);
  }

  if(reason == DLL_PROCESS_DETACH)
  {
    _log_("detach -> %s", g_file_proc);
  }

  return TRUE;
} // DllMain

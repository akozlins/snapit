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
#include <math.h>
#include <limits.h>

#define STRICT 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#define DLL_EXPORT __declspec(dllexport)

#define DX 16
#define DY 16

HHOOK hhook_g = 0;
HINSTANCE hinst_g = 0;

#define _log_ flog

void flog(const char* fmt, ...)
{
  FILE* file = fopen("d:/out.txt", "a+");
  if(!file) return;

  va_list list;
  va_start(list, fmt);
  vfprintf(file, fmt, list);
  va_end( arglist );

  fclose(file);
} // flog

HWND hwnd_g = 0;

size_t rects_n = 0;
RECT rects[64];

BOOL CALLBACK fenum(HWND hwnd, LPARAM lp)
{
  if(rects_n == 64) return FALSE;

  RECT rect;
  if(hwnd == hwnd_g || !IsWindowVisible(hwnd) || !GetWindowRect(hwnd, &rect)) return TRUE;
  rects[rects_n++] = rect;

//  if(lp == 0) EnumChildWindows(hwnd, fenum, 1)

  return TRUE;
} // fenum

LRESULT CALLBACK fproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR data)
{
  if(msg != WM_WINDOWPOSCHANGING) goto ret;
  _log_("fproc: msg = WM_WINDOWPOSCHANGING\n");

  PWINDOWPOS pos = (PWINDOWPOS)lp;
  if(pos->flags & SWP_NOMOVE && pos->flags & SWP_NOSIZE) goto ret;
  _log_("  x = %d, y = %d, w = %d, h = %d\n", pos->x, pos->y, pos->cx, pos->cy);

  RECT rect; // current window position
  if(!GetWindowRect(hwnd, &rect)) goto ret;

  // next windown position
  int l = pos->x, r = pos->x + pos->cx, t = pos->y, b = pos->y + pos->cy;

  // distance to closest window border
  int dl = INT_MAX, dr = INT_MAX, dt = INT_MAX, db = INT_MAX;
  int i; for(i = 0; i < rects_n; i++)
  {
    RECT* rect_ = rects + i;
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

  ret:
  return DefSubclassProc(hwnd, msg, wp, lp);
} // fproc

DLL_EXPORT LRESULT CALLBACK fhook(int code, WPARAM wp, LPARAM lp)
{
  if(code < 0 || wp != 0 || lp == 0) goto ret;

  PCWPSTRUCT cwp = (PCWPSTRUCT)lp;
  HWND hwnd = cwp->hwnd;

  switch(cwp->message)
  {
  case WM_ENTERSIZEMOVE:
    _log_("fhook: msg = WM_ENTERSIZEMOVE\n");
    if(hwnd_g) break;
    _log_("  set subclass: hwnd = %08X, fproc = %08X\n", hwnd, fproc);
    if(SetWindowSubclass(hwnd, fproc, 0, 0))
    {
      hwnd_g = hwnd;
      rects_n = 0;
      EnumWindows(fenum, 0);
    }
    break;
  case WM_EXITSIZEMOVE:
    _log_("fhook: msg = WM_EXITSIZEMOVE\n");
    if(!hwnd_g) break;
    _log_("  remove subclass: hwnd = %08X, fproc = %08X\n", hwnd, fproc);
    if(RemoveWindowSubclass(hwnd, fproc, 0)) hwnd_g = 0;
    break;
  }

  ret:
  return CallNextHookEx(hhook_g, code, wp, lp);
} // fhook

DLL_EXPORT int hook_install()
{
  _log_("set hook ...\n");

  hhook_g = SetWindowsHookEx(WH_CALLWNDPROC, fhook, hinst_g, 0);
  _log_("  hhook = %08X\n", hhook_g);
  if(!hhook_g) return -1;

  return 0;
} // install

DLL_EXPORT int hook_uninstall()
{
  if(!hhook_g) return 0;

  _log_("remove hook ...\n");
  if(!UnhookWindowsHookEx(hhook_g))
  {
    _log_("  fail\n");
    return -1;
  }
  else
  {
    _log_("  success\n");
    hhook_g = 0;
    return 0;
  }
} // uninstall

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
  if(reason == DLL_PROCESS_ATTACH)
  {
    _log_("DLL_PROCESS_ATTACH :: hinst = %08X\n", hinst);

    hinst_g = hinst;
    DisableThreadLibraryCalls(hinst);
  }

  if(reason == DLL_PROCESS_DETACH)
  {
    _log_("DLL_PROCESS_DETACH :: hinst = %08X\n", hinst);

    if(hwnd_g) RemoveWindowSubclass(hwnd_g, fproc, 0);

    hook_uninstall();
  }

  return TRUE;
} // DllMain

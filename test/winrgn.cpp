
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

#define STRICT 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

void main()
{
  HWND hwndDesktop = GetDesktopWindow();
  HWND hwndStart = FindWindowEx(hwndDesktop, NULL, "Button", "Start");

  RECT rectDesktop;
  if(!GetWindowRect(hwndDesktop, &rectDesktop)) return;

  HRGN hrgnDesktop = CreateRectRgn(rectDesktop.left, rectDesktop.top, rectDesktop.right, rectDesktop.bottom);
  HRGN hrgn = CreateRectRgn(0, 0, 0, 0);
  HRGN hrgn_ = CreateRectRgn(0, 0, 0, 0);
  for(HWND hwnd_ = GetTopWindow(NULL); hwnd_ != NULL; hwnd_ = GetWindow(hwnd_, GW_HWNDNEXT))
  {
    if(hwnd_ == hwndStart || hwnd_ == hwndDesktop ||
      !IsWindowVisible(hwnd_) || IsIconic(hwnd_)) continue;
    char buffer[256];
    GetWindowText(hwnd_, buffer, sizeof(buffer));
    printf("GetWindowText = %s\n", buffer);

    RECT rect;
    if(!GetWindowRect(hwnd_, &rect)) continue;

    if(!SetRectRgn(hrgn_, rect.left, rect.top, rect.right, rect.bottom)) continue;
    if(hrgnDesktop)
    {
      if(CombineRgn(hrgn_, hrgn_, hrgnDesktop, RGN_AND) == NULLREGION) continue;
      if(GetRgnBox(hrgn_, &rect) == NULLREGION) continue;
    }
    if(CombineRgn(hrgn_, hrgn_, hrgn, RGN_DIFF) == NULLREGION) continue;
    if(CombineRgn(hrgn, hrgn, hrgn_, RGN_OR) == ERROR) continue;

    printf("RECT :: l = %d, r = %d, t = %d, b = %d\n", rect.left, rect.right, rect.top, rect.bottom);
  }
  DeleteObject(hrgn_);
  DeleteObject(hrgn);
  DeleteObject(hrgnDesktop);
}

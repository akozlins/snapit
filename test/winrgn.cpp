
#include <stdio.h>

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

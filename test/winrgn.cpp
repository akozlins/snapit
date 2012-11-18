
#include <stdio.h>

#include <windows.h>

void main()
{
  HRGN hrgn = CreateRectRgn(0, 0, 0, 0);

  for(HWND hwnd_ = GetTopWindow(NULL); hwnd_ != NULL; hwnd_ = GetWindow(hwnd_, GW_HWNDNEXT))
  {
    if(!IsWindowVisible(hwnd_) || IsIconic(hwnd_)) continue;
    char buffer[256];
    GetWindowText(hwnd_, buffer, sizeof(buffer));
    printf("GetWindowText = %s\n", buffer);

    RECT rect;
    if(GetWindowRect(hwnd_, &rect))
    {
      printf("RECT :: l = %d, r = %d, t = %d, b = %d\n", rect.left, rect.right, rect.top, rect.bottom);

      HRGN hrgn_ = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
      if(CombineRgn(hrgn_, hrgn_, hrgn, RGN_DIFF) == NULLREGION)
      {
        printf("CombineRgn == NULLREGION\n");
      }
      CombineRgn(hrgn, hrgn, hrgn_, RGN_OR);
      DeleteObject(hrgn_);
    }
  }
}

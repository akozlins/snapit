
#include <stdio.h>
#include <math.h>
#include <limits.h>

#define STRICT 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#define DLL_EXPORT __declspec(dllexport)

#define DX 32
#define DY 32

HHOOK hhook = 0;
HINSTANCE hinst_ = 0;

void _log_(const char* fmt, ...)
{
  return;

  FILE* file = fopen("d:/out.txt", "a+");
  if(!file) return;

  va_list list;
  va_start(list, fmt);
  vfprintf(file, fmt, list);
  va_end( arglist );

  fclose(file);
}

typedef struct {
  HWND hwnd;
//  WNDPROC fproc;
  int l, r, t, b;
  int dl, dr, dt, db;
} wnd_node;

wnd_node wnds[16] = { 0 };

wnd_node* wnd_find(HWND hwnd)
{
  int i; for(i = 0; i < 16; i++) if(wnds[i].hwnd == hwnd) return &wnds[i];
  return 0;
}

BOOL CALLBACK fenum(HWND hwnd, LPARAM lp)
{
  wnd_node* node = wnds + lp;
  RECT rect;
  if(hwnd == node->hwnd || !IsWindowVisible(hwnd) || !GetWindowRect(hwnd, &rect)) return TRUE;

  if(node->t < rect.bottom + DY && node->b > rect.top - DY)
  {
    int dl = rect.left - node->l;
    if(abs(dl) < abs(node->dl)) node->dl = dl;
    dl = rect.right - node->l;
    if(abs(dl) < abs(node->dl)) node->dl = dl;

    int dr = rect.left - node->r;
    if(abs(dr) < abs(node->dr)) node->dr = dr;
    dr = rect.right - node->r;
    if(abs(dr) < abs(node->dr)) node->dr = dr;
  }

  if(node->l < rect.right + DX && node->r > rect.left - DX)
  {
    int dt = rect.top - node->t;
    if(abs(dt) < abs(node->dt)) node->dt = dt;
    dt = rect.bottom - node->t;
    if(abs(dt) < abs(node->dt)) node->dt = dt;

    int db = rect.top - node->b;
    if(abs(db) < abs(node->db)) node->db = db;
    db = rect.bottom - node->b;
    if(abs(db) < abs(node->db)) node->db = db;
  }

  return TRUE;
}

LRESULT CALLBACK fproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR data)
{
  switch(msg)
  {
  case WM_WINDOWPOSCHANGING:
    _log_("fproc: msg = WM_WINDOWPOSCHANGING\n");

    PWINDOWPOS pos = (PWINDOWPOS)lp;
    if(pos->flags & SWP_NOMOVE && pos->flags & SWP_NOSIZE) break;
    _log_("  x = %d, y = %d, w = %d, h = %d\n", pos->x, pos->y, pos->cx, pos->cy);

    RECT rect; // current window position
    if(!GetWindowRect(hwnd, &rect)) break;

    wnd_node* node = wnd_find(hwnd);
    if(!node) break;

    node->l = pos->x;
    node->r = pos->x + pos->cx;
    node->t = pos->y;
    node->b = pos->y + pos->cy;

    node->dl = node->dr = node->dt = node->db = INT_MAX;
    EnumWindows(fenum, node - wnds);
    _log_("  dl = %d, dr = %d, dt = %d, db = %d\n", node->dl, node->dr, node->dt, node->db);

    if(!(pos->flags & SWP_NOMOVE) &&
      pos->cx == (rect.right - rect.left) && pos->cy == (rect.bottom - rect.top) &&
      (pos->x != rect.left || pos->y != rect.top))
    {
      int dx = node->dl;
      if(abs(node->dr) < abs(dx)) dx = node->dr;
      int dy = node->dt;
      if(abs(node->db) < abs(dy)) dy = node->db;

      if(abs(dx) < DX) pos->x = node->l + dx;
      if(abs(dy) < DY) pos->y = node->t + dy;
    }
    else if(!(pos->flags & SWP_NOSIZE))
    {
      if(pos->cx != (node->r - node->l))
      {
      }
      if(pos->cy != (node->b - node->t))
      {
      }
    }

    break;
  }

  ret:
  return DefSubclassProc(hwnd, msg, wp, lp);
}

DLL_EXPORT LRESULT CALLBACK fhook(int code, WPARAM wp, LPARAM lp)
{
  if(code < 0 || wp != 0 || lp == 0) goto ret;

  PCWPSTRUCT cwp = (PCWPSTRUCT)lp;
  HWND hwnd = cwp->hwnd;

  wnd_node* node;
  switch(cwp->message)
  {
  case WM_ENTERSIZEMOVE:
    _log_("fhook: msg = WM_ENTERSIZEMOVE\n");
    if(wnd_find(hwnd) || !(node = wnd_find(0))) break;

    node->hwnd = hwnd;
    _log_("  set subclass: hwnd = %08X, fproc = %08X\n", hwnd, fproc);
    SetWindowSubclass(hwnd, fproc, 0, 0);
    break;
  case WM_EXITSIZEMOVE:
    _log_("fhook: msg = WM_EXITSIZEMOVE\n");
    if(!(node = wnd_find(hwnd))) break;

    _log_("  remove subclass: hwnd = %08X, fproc = %08X\n", hwnd, fproc);
    RemoveWindowSubclass(hwnd, fproc, 0);
    memset(node, 0, sizeof(wnd_node));
    break;
  }

  ret:
  return CallNextHookEx(hhook, code, wp, lp);
}

DLL_EXPORT int install()
{
  _log_("set hook ...\n");

  hhook = SetWindowsHookEx(WH_CALLWNDPROC, fhook, hinst_, 0);
  _log_("  hhook = %08X\n", hhook);
  if(!hhook) return -1;

  return 0;
}

DLL_EXPORT int uninstall()
{
  if(!hhook) return 0;

  _log_("remove hook ...\n");
  if(!UnhookWindowsHookEx(hhook))
  {
    _log_("  fail\n");
    return -1;
  }
  else
  {
    _log_("  success\n");
    hhook = 0;
    return 0;
  }
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
  if(reason == DLL_PROCESS_ATTACH)
  {
    _log_("DLL_PROCESS_ATTACH :: hinst = %08X\n", hinst);

    hinst_ = hinst;
    DisableThreadLibraryCalls(hinst);
  }

  if(reason == DLL_PROCESS_DETACH)
  {
    _log_("DLL_PROCESS_DETACH :: hinst = %08X\n", hinst);

    int i; for(i = 0; i < 16; i++) if(wnds[i].hwnd) RemoveWindowSubclass(wnds[i].hwnd, fproc, 0);

    uninstall();
  }

  return TRUE;
}

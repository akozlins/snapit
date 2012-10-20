
#include <stdio.h>

#include <windows.h>

#define DLL_EXPORT __declspec(dllexport)

HHOOK hhook = 0;
HINSTANCE hinst_ = 0;

void log(const char* fmt, ...)
{
  FILE* file = fopen("d:/out.txt", "a+");
  if(file == 0) return;

  va_list list;
  va_start(list, fmt);
  vfprintf(file, fmt, list);
  va_end( arglist );

  fclose(file);
}

typedef struct {
  HWND hwnd;
  WNDPROC fproc;
} hwnd_proc_node;

hwnd_proc_node hwnd_proc[16] = { 0 };

hwnd_proc_node* hwnd_proc_find(HWND hwnd)
{
  int i;
  for(i = 0; i < 16; i++) if(hwnd_proc[i].hwnd == hwnd) return &hwnd_proc[i];
  return 0;
}

LRESULT CALLBACK fproc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam, UINT_PTR id, DWORD_PTR data)
{
  if(msg == WM_MOVE)
  {
    log("WNDPROC: WM_MOVE\n");
  }

  return DefSubclassProc(hwnd, msg, wparam, lparam);
}

DLL_EXPORT LRESULT CALLBACK fhook(int code, WPARAM wparam, LPARAM lparam)
{
  if(code < 0 || wparam != 0 || lparam == 0) goto ret;

  PCWPSTRUCT cwp = (PCWPSTRUCT)lparam;
  HWND hwnd = cwp->hwnd;
  UINT msg = cwp->message;

  if(msg == WM_ENTERSIZEMOVE)
  {
    log("WM_ENTERSIZEMOVE\n");
    hwnd_proc_node* node;
    if(hwnd_proc_find(hwnd) == 0 && (node = hwnd_proc_find(0)) != 0)
    {
      node->hwnd = hwnd;
      log("install subclass: %08X / %08X\n", hwnd, fproc);
      SetWindowSubclass(hwnd, fproc, 0, 0);
    }
  }
  if(msg == WM_EXITSIZEMOVE)
  {
    log("WM_EXITSIZEMOVE\n");
    hwnd_proc_node* node = hwnd_proc_find(hwnd);
    if(node != 0)
    {
      log("uninstall subclass: %08X\n", hwnd);
      RemoveWindowSubclass(hwnd, fproc, 0);
      node->hwnd = 0;
    }
  }
  if(msg == WM_SYSCOMMAND)
  {
    log("WM_SYSCOMMAND\n");
    log("w = %08X / l = %08X\n", cwp->wParam, cwp->lParam);
    int x = cwp->lParam & 0xFFFF;
    int y = cwp->lParam >> 16;
    switch(cwp->wParam & 0xFFF0)
    {
    case SC_MOVE:
      break;
    case SC_SIZE:
      break;
    default:
      printf("");
    }
  }

  ret:
  return CallNextHookEx(hhook, code, wparam, lparam);
}

DLL_EXPORT int install()
{
  log("install hook\n");
  hhook = SetWindowsHookEx(WH_CALLWNDPROC, fhook, hinst_, 0);
  if(hhook == 0) return -1;
  return 0;
}

DLL_EXPORT int uninstall()
{
  if(hhook == 0) return -1;

  log("uninstall hook\n");
  if(UnhookWindowsHookEx(hhook) != 0)
  {
    hhook = 0;
    return 0;
  }
  return -1;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD reason, LPVOID reserved)
{
  if(reason == DLL_PROCESS_ATTACH)
  {
    hinst_ = hinst;
    DisableThreadLibraryCalls(hinst);

    log("DLL_PROCESS_ATTACH / %08X\n", hinst);
  }

  if(reason == DLL_PROCESS_DETACH)
  {
    uninstall();
  }

  return TRUE;
}

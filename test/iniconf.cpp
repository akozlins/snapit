
#include <stdio.h>
#include <stdlib.h>

#define STRICT 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#include <psapi.h>

#include "../iniconf.h"

void main()
{
  char fname[256];
  HANDLE hproc = GetCurrentProcess();
  int n = GetModuleFileNameEx(hproc, 0, fname, sizeof(fname));
  CloseHandle(hproc);
  while(fname[n] != '\\') n--;
  strcpy_s(fname + n, sizeof(fname) - n, "\\iniconf.ini");

  iniconf conf;
  conf.ctor(fname);

  for(int i = 0; i < 16; i++)
  {
    printf("%s = %s\n", conf.map[i][0], conf.map[i][1]);
  }

  conf.dtor();
}

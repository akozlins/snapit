
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

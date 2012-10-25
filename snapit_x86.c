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

int hook_install();
int hook_uninstall();

void main(int argc, char **argv)
{
  printf("usage: %s -t [sec]\n", argv[0]);

  if(argc >= 2 && strcmp("-t", argv[1]) == 0)
  {
    hook_install();

    int t = 30;
    if(argc == 3) t = atoi(argv[2]);
    while(t-- > 0)
    {
      Sleep(1000);
      printf("%4d\r", t);
    }
    printf("\n");

    hook_uninstall();

    return;
  }

  if(argc == 2 && strcmp("-i", argv[1]) == 0)
  {
    hook_install();

    return;
  }

  if(argc == 2 && strcmp("-u", argv[1]) == 0)
  {
    hook_uninstall();

    return;
  }
}

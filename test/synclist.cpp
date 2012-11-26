
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

volatile long g_lock_log = 0;

struct log_node
{
  log_node* next;
  int data;
};
log_node* head = 0;

volatile long stop = 0;

DWORD WINAPI write(LPVOID data)
{
  int d = data ? *(int*)data : 1;
  int s = 0;

  while(!stop)
  {
    log_node* node = (log_node*)malloc(sizeof(log_node));
    node->data = d;

    while(InterlockedExchange(&g_lock_log, 1) == 1);
    node->next = head;
    head = node;
    InterlockedExchange(&g_lock_log, 0);

    s += d;
  }

  printf("write: s = %d\n", s);

  return 0;
}

DWORD WINAPI read(LPVOID data)
{
  int s = 0;

  log_node* node;
  while(!stop || head)
  {
    while(InterlockedExchange(&g_lock_log, 1) == 1);
    if(!head) { g_lock_log = 0; continue; }
    node = head;
    head = node->next;
    InterlockedExchange(&g_lock_log, 0);

    s += node->data;
    free(node);
  }

  printf("read: s = %d\n", s);

  return 0;
}

void main()
{
  int n = 0;
  HANDLE handles[32];

  handles[n++] = CreateThread(0, 0, write, 0, 0, 0);
  handles[n++] = CreateThread(0, 0, write, 0, 0, 0);
  handles[n++] = CreateThread(0, 0, write, 0, 0, 0);
  handles[n++] = CreateThread(0, 0, write, 0, 0, 0);

  Sleep(500);

  handles[n++] = CreateThread(0, 0, read, 0, 0, 0);
  handles[n++] = CreateThread(0, 0, read, 0, 0, 0);
  handles[n++] = CreateThread(0, 0, read, 0, 0, 0);
  handles[n++] = CreateThread(0, 0, read, 0, 0, 0);
  handles[n++] = CreateThread(0, 0, read, 0, 0, 0);
  handles[n++] = CreateThread(0, 0, read, 0, 0, 0);
  handles[n++] = CreateThread(0, 0, read, 0, 0, 0);
  handles[n++] = CreateThread(0, 0, read, 0, 0, 0);

  for(int i = 0; i < 10; i++)
  {
    printf(".");
    Sleep(500);
  }
  printf("\n");

  stop = 1;

  WaitForMultipleObjects(n, handles, TRUE, 100000);
  for(int i = 0; i < n; i++) CloseHandle(handles[i]);
}

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

#define UNICODE 1

#define STRICT 1
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>

#include <winsvc.h>

void service_install()
{
  wchar_t path[256];
  DWORD size = GetModuleFileName(NULL, path, 256);

  wchar_t* name = L"'snapit' x86 service";

  SC_HANDLE manager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
  if(manager == 0)
  {
    MessageBox(0, L"OpenSCManager failed.", L"Error", MB_OK);
    return;
  }

  SC_HANDLE service = CreateService(manager, name, name, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS | SERVICE_INTERACTIVE_PROCESS, SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, path, NULL, NULL, NULL, NULL, NULL);

  if(service == 0)
  {
    MessageBox(0, L"CreateService failed.", L"Error", MB_OK);
  }
  else
  {
    CloseServiceHandle(service);
  }

  CloseServiceHandle(manager);
}

void service_uninstall()
{
  wchar_t* name = L"'snapit' x86 service";

  SC_HANDLE manager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if(manager ==0)
  {
    MessageBox(0, L"OpenSCManager failed.", L"Error", MB_OK);
    return;
  }

  SC_HANDLE service = OpenService(manager, name, SERVICE_ALL_ACCESS);
  if(service == 0)
  {
    MessageBox(0, L"OpenService failed.", L"Error", MB_OK);
  }
  else
  {
    if(!DeleteService(service))
    {
      MessageBox(0, L"DeleteService failed.", L"Error", MB_OK);
    }
    CloseServiceHandle(service);
  }

  CloseServiceHandle(manager);
}

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle;

BOOL reportStatus(DWORD currentState)
{
  if(currentState == SERVICE_START_PENDING) serviceStatus.dwControlsAccepted = 0;
  else serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

  serviceStatus.dwCurrentState = currentState;
  serviceStatus.dwWin32ExitCode = NO_ERROR;
  serviceStatus.dwWaitHint = 100;

  if(currentState == SERVICE_RUNNING || currentState == SERVICE_STOPPED) serviceStatus.dwCheckPoint = 0;
  else serviceStatus.dwCheckPoint++;

  return SetServiceStatus(serviceStatusHandle, &serviceStatus);
}

void WINAPI serviceControl(DWORD controlCode)
{
  if(controlCode == SERVICE_CONTROL_STOP)
  {
    reportStatus(SERVICE_STOP_PENDING);
    hook_uninstall();
    reportStatus(SERVICE_STOPPED);
  }
}

void WINAPI serviceMain(DWORD argc, LPTSTR *argv)
{
  serviceStatusHandle = RegisterServiceCtrlHandler(L"", serviceControl);
  if(!serviceStatusHandle) return;

  serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
  serviceStatus.dwServiceSpecificExitCode = 0;

  reportStatus(SERVICE_START_PENDING);
  hook_install();
  reportStatus(SERVICE_RUNNING);

  return;
}

void _cdecl main(int argc, char **argv)
{
  if(argc == 2 && _stricmp("-t", argv[1]) == 0)
  {
    hook_install();
    Sleep(30000);
    hook_uninstall();

    return;
  }

  if(argc == 2 && _stricmp("-i", argv[1]) == 0)
  {
    service_install();

    return;
  }

  if(argc == 2 && _stricmp("-u", argv[1]) == 0)
  {
    service_uninstall();

    return;
  }

  SERVICE_TABLE_ENTRY dispatchTable[] = { { L"", (LPSERVICE_MAIN_FUNCTION)serviceMain }, { NULL, NULL } };
  StartServiceCtrlDispatcher(dispatchTable);
}

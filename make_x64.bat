
call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" amd64

@set CLOPTS=/nologo /W3 /MD /O2 /D "NDEBUG" /c
@set LINKOPTS=/nologo /RELEASE /INCREMENTAL:NO /OPT:REF /OPT:ICF /NODEFAULTLIB msvcrt.lib kernel32.lib

del hook.obj
cl %CLOPTS% /D "WIN64" hook.cpp
link %LINKOPTS% user32.lib gdi32.lib comctl32.lib psapi.lib /DLL /out:hook_x64.dll hook.obj /MACHINE:X64

del snapit.obj
cl %CLOPTS% /D "WIN64" snapit.cpp
link %LINKOPTS% user32.lib gdi32.lib shell32.lib hook_x64.lib /out:snapit_x64.exe snapit.obj /MACHINE:X64 /SUBSYSTEM:WINDOWS

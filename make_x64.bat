
call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat" amd64

@set CLOPTS=/nologo /W3 /MD /O2 /D "NDEBUG" /D "WIN64" /c
@set LINKOPTS=/nologo /RELEASE /INCREMENTAL:NO /OPT:REF /OPT:ICF /NODEFAULTLIB msvcrt.lib kernel32.lib /MACHINE:X64

cl %CLOPTS% /Tp hook_x64.c
link %LINKOPTS% user32.lib gdi32.lib comctl32.lib psapi.lib /DLL /out:hook_x64.dll hook_x64.obj

cl %CLOPTS% /Tp snapit_x64.c
link %LINKOPTS% user32.lib gdi32.lib shell32.lib hook_x64.lib /out:snapit_x64.exe snapit_x64.obj /SUBSYSTEM:WINDOWS

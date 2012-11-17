
call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat"

@set CLOPTS=/nologo /W3 /MD /O2 /D "NDEBUG" /D "WIN32" /c
@set LINKOPTS=/nologo /RELEASE /INCREMENTAL:NO /OPT:REF /OPT:ICF /NODEFAULTLIB msvcrt.lib kernel32.lib /MACHINE:X86

cl %CLOPTS% /Tp hook_x86.c
link %LINKOPTS% user32.lib comctl32.lib psapi.lib /DLL /out:hook_x86.dll hook_x86.obj

cl %CLOPTS% /Tp snapit_x86.c
link %LINKOPTS% user32.lib gdi32.lib shell32.lib hook_x86.lib /out:snapit_x86.exe snapit_x86.obj /SUBSYSTEM:WINDOWS

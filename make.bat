
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"

@set CLOPTS=/nologo /W3 /MD /c /O1 /EHa /D "NDEBUG" /D "WIN32"
@set LINKOPTS=/nologo /RELEASE /INCREMENTAL:NO /OPT:REF /OPT:ICF /NODEFAULTLIB msvcrt.lib kernel32.lib
@rem /SUBSYSTEM:WINDOWS /MACHINE:X86

cl %CLOPTS% /Tp hook_x86.c
link %LINKOPTS% user32.lib comctl32.lib psapi.lib /DLL /out:hook_x86.dll hook_x86.obj

cl %CLOPTS% /Tp snapit_x86.c
link %LINKOPTS% hook_x86.lib /out:snapit_x86.exe snapit_x86.obj

rem "c:/develop/tcc/tiny_impdef" comctl32.dll -o comctl32.def
rem "c:/develop/tcc/tcc" -shared -L. -lcomctl32 -o hook_x86.dll hook_x86.c

rem "c:/develop/tcc/tcc" -L. -lhook_x86 -o snapit_x86.exe snapit_x86.c

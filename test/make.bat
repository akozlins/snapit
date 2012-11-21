
call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat"

@set CLOPTS=/nologo /W3 /MD /O2 /D "NDEBUG" /c
@set LINKOPTS=/nologo /RELEASE /INCREMENTAL:NO /OPT:REF /OPT:ICF /NODEFAULTLIB msvcrt.lib kernel32.lib

cl %CLOPTS% /D "WIN32" winrgn.cpp
link %LINKOPTS% user32.lib gdi32.lib shell32.lib /out:winrgn.exe winrgn.obj /MACHINE:X86 /SUBSYSTEM:CONSOLE

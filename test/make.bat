
call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat"

@set CLOPTS=/nologo /W3 /MD /O2 /D "NDEBUG" /D "WIN32" /c
@set LINKOPTS=/nologo /RELEASE /INCREMENTAL:NO /OPT:REF /OPT:ICF /NODEFAULTLIB msvcrt.lib kernel32.lib /MACHINE:X86

cl %CLOPTS% /Tp winrgn.cpp
link %LINKOPTS% user32.lib gdi32.lib shell32.lib /out:winrgn.exe winrgn.obj /SUBSYSTEM:CONSOLE

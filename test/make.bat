
@call "%VS100COMNTOOLS%\..\..\vc\vcvarsall.bat"

@set CLOPTS=/nologo /W3 /MD /O2 /D "NDEBUG" /c
@set LINKOPTS=/nologo /RELEASE /INCREMENTAL:NO /OPT:REF /OPT:ICF /NODEFAULTLIB msvcrt.lib kernel32.lib

@set fname=winrgn
@cl %CLOPTS% /D "WIN32" %fname%.cpp
@link %LINKOPTS% user32.lib gdi32.lib shell32.lib /out:%fname%.exe %fname%.obj /MACHINE:X86 /SUBSYSTEM:CONSOLE

@set fname=synclist
@cl %CLOPTS% /D "WIN32" %fname%.cpp
@link %LINKOPTS% user32.lib gdi32.lib shell32.lib /out:%fname%.exe %fname%.obj /MACHINE:X86 /SUBSYSTEM:CONSOLE

@set fname=iniconf
@cl %CLOPTS% /D "WIN32" %fname%.cpp
@link %LINKOPTS% user32.lib gdi32.lib shell32.lib psapi.lib /out:%fname%.exe %fname%.obj /MACHINE:X86 /SUBSYSTEM:CONSOLE

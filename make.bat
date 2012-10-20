
"c:/develop/tcc/tcc" -shared -o hook_x86.dll hook_x86_dll.c
"c:/develop/tcc/tcc" -luser32 -L. -lhook_x86 -o test.exe test.c

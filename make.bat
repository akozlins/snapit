
@echo "make hook_x86.dll ..."
"c:/develop/tcc/tiny_impdef" comctl32.dll -o comctl32.def
"c:/develop/tcc/tcc" -shared -L. -lcomctl32 -o hook_x86.dll hook_x86_dll.c

@echo "make service_x86.exe ..."
"c:/develop/tcc/tiny_impdef" advapi32.dll -o advapi32.def
"c:/develop/tcc/tcc" -luser32 -L. -ladvapi32 -lhook_x86 -o service_x86.exe service_x86_exe.c

@echo "make test.exe ..."
"c:/develop/tcc/tcc" -luser32 -L. -lhook_x86 -o test.exe test.c

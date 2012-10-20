
@echo "make hook_x86.dll ..."
"c:/develop/tcc/tiny_impdef" comctl32.dll -o lib/comctl32.def
"c:/develop/tcc/tcc" -shared -Llib -lcomctl32 -o hook_x86.dll hook_x86_dll.c

@echo "make test.exe ..."
"c:/develop/tcc/tcc" -luser32 -L. -lhook_x86 -o test.exe test.c

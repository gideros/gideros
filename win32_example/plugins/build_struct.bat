gcc -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/struct/source/struct.c

g++ -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/struct/source/struct_stub.cpp 

g++ -o struct.dll -shared struct.o struct_stub.o ../lua.dll

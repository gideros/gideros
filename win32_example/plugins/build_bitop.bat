:: -----
:: BITOP
:: -----

gcc -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/bitop/source/bit.c

g++ -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/bitop/source/bit_stub.cpp 

g++ -o bitop.dll -shared bit.o bit_stub.o ../lua.dll

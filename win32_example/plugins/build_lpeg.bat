gcc -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/lpeg/source/*.c

g++ -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/lpeg/source/*.cpp 

g++ -o lpeg.dll -shared lpvm.o lpcap.o lptree.o lpcode.o lpprint.o lpeg_stub.o ../lua.dll

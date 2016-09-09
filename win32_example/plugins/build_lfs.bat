:: ---
:: LFS
:: ---

gcc -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/lfs/source/*.c

g++ -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/lfs/source/*.cpp 

g++ -o lfs.dll -shared lfs.o lfs_stub.o ../lua.dll ../gvfs.dll

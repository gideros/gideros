:: ------
:: SQLITE
:: ------

gcc -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/lsqlite3/source/*.c

g++ -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/lsqlite3/source/*.cpp 

g++ -o lsqlite3.dll -shared sqlite3.o lsqlite3.o lsqlite3_stub.o ../lua.dll ../gvfs.dll

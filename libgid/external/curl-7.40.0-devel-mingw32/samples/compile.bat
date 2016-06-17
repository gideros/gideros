gcc -I ..\include -o simple.exe simple.c ..\lib\libcurldll.a
gcc -I ..\include -o https.exe https.c ..\lib\libcurldll.a

gcc -I ..\include -I D:\gideros_gideros\gideros\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\include ^
-o multithread.exe multithread.c ..\lib\libcurldll.a ^
D:\gideros_gideros\gideros\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\lib\x86\libpthreadGC2.a

g++ -I D:\gideros_gideros\gideros\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\include ^
-o ptest.exe ptest.cpp ^
D:\gideros_gideros\gideros\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\lib\x86\libpthreadGC2.a

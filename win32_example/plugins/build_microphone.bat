g++ -I ..\..\libgid\external\openal-soft-1.13\include\AL ^
-I ..\..\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\include ^
-I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/microphone/source/*.cpp 

g++ -o microphone.dll -shared gmicrophone-openal.o gsoundencoder-wav.o gmicrophonebinder.o ^
../lua.dll ../gid.dll ../gideros.dll ../gvfs.dll ^
..\..\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\lib\x86\libpthreadGC2.a ^
..\..\libgid\external\openal-soft-1.13\build\mingw48_32\libOpenAL32.dll.a

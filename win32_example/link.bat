g++ -o win32.exe *.o ^
..\libgid\external\freetype-2.4.12\build\mingw48_32\libfreetype.a ^
..\libgid\external\jpeg-9\build\mingw48_32\libjpeg.a ^
..\libgid\external\libpng-1.6.2\build\mingw48_32\libpng.a ^
..\libgid\external\mpg123-1.15.3\lib\mingw48_32\libmpg123.a ^
..\libgid\external\openal-soft-1.13\build\mingw48_32\libOpenAL32.dll.a ^
..\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\lib\x86\libpthreadGC2.a ^
..\libgid\external\zlib-1.2.8\build\mingw48_32\libzlibx.a ^
-L"../libgid/external/glew-1.10.0/lib/mingw48_32" -lglew32 ^
-L"../libgid/release" -lgid ^
-L"../libgvfs/release" -lgvfs ^
-L"../lua/release" -llua ^
-L"../libgideros/release" -lgideros ^
-L"../libpystring/release" -lpystring

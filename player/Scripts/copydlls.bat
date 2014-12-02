REM bat to create a folder to execute the player
REM some files are required to execute, build the dependency libraries first
REM this bat file should run at main folder

REM create the temp dirs to store the .dll and .exe files

mkdir builded
cd builded
mkdir release
mkdir debug
cd ..

REM copy the required .dlls for release version

cp ..\libgid\release\gid.dll builded\release
cp ..\libgvfs\release\gvfs.dll builded\release
cp ..\lua\release\lua.dll builded\release
cp ..\libgideros\release\gideros.dll builded\release
cp ..\libpystring\release\pystring.dll builded\release
cp ..\libgid\external\zlib-1.2.8\build\mingw\zlib.dll builded\release
cp ..\libgid\external\glew-1.10.0\lib\mingw\glew32.dll builded\release
cp ..\libgid\external\openal-soft-1.13\build\mingw\OpenAL32.dll builded\release
cp ..\libgid\external\pthreads-w32-2-8-0-release\Pre-built.2\lib\pthreadGC2.dll builded\release

REM copy the required .dlls for debug version

cp ..\libgid\release\gid.dll builded\debug
cp ..\libgvfs\release\gvfs.dll builded\debug
cp ..\lua\release\lua.dll builded\debug
cp ..\libgideros\release\gideros.dll builded\debug
cp ..\libpystring\release\pystring.dll builded\debug
cp ..\libgid\external\zlib-1.2.8\build\mingw\zlib.dll builded\debug
cp ..\libgid\external\glew-1.10.0\lib\mingw\glew32.dll builded\debug
cp ..\libgid\external\openal-soft-1.13\build\mingw\OpenAL32.dll builded\debug
cp ..\libgid\external\pthreads-w32-2-8-0-release\Pre-built.2\lib\pthreadGC2.dll builded\debug

REM copy the luajit library, debug and release is the samme)

cp ..\luajit\src\lua51.dll builded\release\lua.dll
cp ..\luajit\src\lua51.dll builded\debug\lua.dll

REM copy the builded player executable, debug and release

cp release\GiderosPlayer.exe builded\release
cp debug\GiderosPlayer.exe builded\debug

REM comment to finish with return 0 on qtcreator
pushd \
call C:\Qt\Qt5.3.2\5.3\mingw482_32\bin\qtenv2.bat
popd

cd ..

cd libpystring
qmake libpystring.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
cd ..

cd libgvfs
qmake libgvfs.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
cd ..

cd libgid
qmake libgid_qt5.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
cd ..

cd lua
qmake lua.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
cd ..

cd libgideros
qmake libgideros.pro
mingw32-make.exe clean
mingw32-make.exe release
mingw32-make.exe release
cd ..

rmdir /s /q Sdk
mkdir Sdk
cd Sdk
mkdir include
copy ..\libgideros\gideros.h include
copy ..\libgideros\gplugin.h include
copy ..\libgideros\gproxy.h include
copy ..\libgideros\greferenced.h include
copy ..\libgideros\gexport.h include
copy ..\libgvfs\gfile.h include
copy ..\lua\src\lua.h include
copy ..\lua\src\luaconf.h include
copy ..\lua\src\lualib.h include
copy ..\lua\src\lauxlib.h include
copy ..\libgid\include\gglobal.h include
copy ..\libgvfs\gpath.h include
copy ..\libgid\include\glog.h include
copy ..\libgid\include\gapplication.h include
copy ..\libgid\include\gevent.h include
mkdir include\AL
copy ..\libgid\external\openal-soft-1.13\include\AL\al.h include\AL
copy ..\libgid\external\openal-soft-1.13\include\AL\alc.h include\AL
copy ..\libgid\external\openal-soft-1.13\include\AL\alext.h include\AL

mkdir lib
mkdir lib\desktop
copy ..\libgideros\release\libgideros.a lib\desktop
copy ..\lua\release\liblua.a lib\desktop
copy ..\libgid\release\libgid.a lib\desktop
copy ..\libgvfs\release\libgvfs.a lib\desktop
copy ..\libgid\external\openal-soft-1.13\build\mingw48_32\libOpenAL32.dll.a lib\desktop
cd ..

cd scripts


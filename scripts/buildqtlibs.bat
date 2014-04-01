pushd \
call C:\Qt\1.2.1\Desktop\Qt\4.8.1\mingw\bin\qtenv2.bat
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
qmake libgid_mingw48_32.pro
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

rm -rf Sdk
mkdir Sdk
cd Sdk
mkdir include
cp ..\libgideros\gideros.h include
cp ..\libgideros\gplugin.h include
cp ..\libgideros\gproxy.h include
cp ..\libgideros\greferenced.h include
cp ..\libgideros\gexport.h include
cp ..\libgvfs\gfile.h include
cp ..\lua\src\lua.h include
cp ..\lua\src\luaconf.h include
cp ..\lua\src\lualib.h include
cp ..\lua\src\lauxlib.h include
cp ..\libgid\include\gglobal.h include
cp ..\libgvfs\gpath.h include
cp ..\libgid\include\glog.h include
cp ..\libgid\include\gapplication.h include
cp ..\libgid\include\gevent.h include
cp -R ..\libgid\external\openal-soft-1.13\include\AL include

mkdir lib
mkdir lib\desktop
cp ..\libgideros\release\libgideros.a lib\desktop
cp ..\lua\release\liblua.a lib\desktop
cp ..\libgid\release\libgid.a lib\desktop
cp ..\libgvfs\release\libgvfs.a lib\desktop
cp ..\libgid\external\openal-soft-1.13\build\mingw\libOpenAL32.dll.a lib\desktop
cd ..

cd scripts



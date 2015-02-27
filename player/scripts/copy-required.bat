rm -rf ..\release\platforms\*
mkdir ..\release\platforms
cp %QT_DIR%\plugins\platforms\qminimal.dll ..\release\platforms
cp %QT_DIR%\plugins\platforms\qoffscreen.dll ..\release\platforms
cp %QT_DIR%\plugins\platforms\qwindows.dll ..\release\platforms

cp %QT_DIR%\bin\icudt53.dll ..\release
cp %QT_DIR%\bin\icuin53.dll ..\release
cp %QT_DIR%\bin\icuuc53.dll ..\release
cp %QT_DIR%\bin\libgcc_s_dw2-1.dll ..\release
cp %QT_DIR%\bin\libstdc++-6.dll ..\release
cp %QT_DIR%\bin\libwinpthread-1.dll ..\release
cp %QT_DIR%\bin\Qt5Core.dll ..\release
cp %QT_DIR%\bin\Qt5Gui.dll ..\release
cp %QT_DIR%\bin\Qt5Network.dll ..\release
cp %QT_DIR%\bin\Qt5OpenGL.dll ..\release
cp %QT_DIR%\bin\Qt5PrintSupport.dll ..\release
cp %QT_DIR%\bin\Qt5Widgets.dll ..\release
cp %QT_DIR%\bin\Qt5Xml.dll ..\release

cp ..\libgid\release\gid.dll ..\release
cp ..\libgideros\release\gideros.dll ..\release
cp ..\libgvfs\release\gvfs.dll ..\release
cp ..\lua\release\lua.dll ..\release
cp ..\libpystring\release\pystring.dll ..\release

cp ..\libgid\external\glew-1.10.0\lib\mingw\glew32.dll ..\release
cp ..\libgid\external\openal-soft-1.13\build\mingw\OpenAL32.dll ..\release

REM zlib not used for now
REM cp ..\libgid\external\zlib-1.2.8\build\mingw\zlib.dll ..\release

REM luajit not working for now
REM cp ..\luajit\src\lua51.dll ..\release\lua.dll
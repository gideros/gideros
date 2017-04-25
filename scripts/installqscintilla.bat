SET BUILD_DIR=%CD%
pushd \
call C:\Qt\Qt5.6.0\5.6\mingw49_32\bin\qtenv2.bat
popd

cd ..\build

cd QScintilla-gpl-2.8.4\Qt4Qt5
ren InputMethod.cpp InputMethod.bak
cscript %BUILD_DIR%\commentlines.vbs InputMethod.bak 33 34 InputMethod.cpp
qmake qscintilla.pro
mingw32-make clean
mingw32-make release
mingw32-make install
cd ..\..

cd ..\scripts

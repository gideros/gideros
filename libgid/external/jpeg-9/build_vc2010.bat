pushd \
call "C:\Qt\5.0.2\5.0.2\msvc2010_opengl\bin\qtenv2.bat"
call "C:\Program Files\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
popd
qmake jpeg.pro
nmake /f Makefile.Release clean
nmake /f Makefile.Release
nmake /f Makefile.Debug clean
nmake /f Makefile.Debug


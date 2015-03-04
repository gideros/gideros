pushd \
call C:\Qt\1.2.1\Desktop\Qt\4.8.1\mingw\bin\qtenv2.bat
popd

cd ..

cd texturepacker
qmake texturepacker.pro
mingw32-make.exe release
cd ..

cd fontcreator
qmake fontcreator.pro
mingw32-make.exe release
cd ..

cd ui
qmake ui.pro
mingw32-make.exe release
cd ..

cd player
qmake player_qt4.pro
mingw32-make.exe release
cd ..

cd gdrdeamon
qmake gdrdeamon.pro
mingw32-make.exe release
cd ..

cd gdrbridge
qmake gdrbridge.pro
mingw32-make.exe release
cd ..

cd gdrexport
qmake gdrexport.pro
mingw32-make.exe release
cd ..

cd scripts

pushd \
call C:\Qt\1.2.1\Desktop\Qt\4.8.1\mingw\bin\qtenv2.bat
popd

cd ..

cd texturepacker
mingw32-make.exe clean
qmake texturepacker.pro
mingw32-make.exe clean
cd ..

cd fontcreator
mingw32-make.exe clean
qmake fontcreator.pro
mingw32-make.exe clean
cd ..

cd ui
mingw32-make.exe clean
qmake ui.pro
mingw32-make.exe clean
cd ..

cd player
mingw32-make.exe clean
qmake player_qt4.pro
mingw32-make.exe clean
cd ..

cd gdrdeamon
mingw32-make.exe clean
qmake gdrdeamon.pro
mingw32-make.exe clean
cd ..

cd gdrbridge
mingw32-make.exe clean
qmake gdrbridge.pro
mingw32-make.exe clean
cd ..

cd gdrexport
mingw32-make.exe clean
qmake gdrexport.pro
mingw32-make.exe clean
cd ..

cd scripts

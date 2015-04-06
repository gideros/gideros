pushd \
call C:\Qt\Qt5.3.2\5.3\mingw482_32\bin\qtenv2.bat
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
qmake player_qt5.pro
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

pushd \
call C:\Qt\Qt5.3.2\5.3\mingw482_32\bin\qtenv2.bat
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
qmake player_qt5.pro
mingw32-make.exe clean
cd ..

cd licensemanager
mingw32-make.exe clean
qmake licensemanager.pro
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

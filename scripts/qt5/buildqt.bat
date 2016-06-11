pushd \
call C:\Qt\Qt5.6.0\5.6\mingw49_32\bin\qtenv2.bat
popd

cd ..

cd texturepacker
qmake "CONFIG+=warn_off" texturepacker.pro
mingw32-make.exe release
cd ..

cd fontcreator
qmake "CONFIG+=warn_off" fontcreator.pro
mingw32-make.exe release
cd ..

cd ui
qmake "CONFIG+=warn_off" ui.pro
mingw32-make.exe release
cd ..

cd player
qmake "CONFIG+=warn_off" player_qt5.pro
mingw32-make.exe release
cd ..

cd desktop
qmake "CONFIG+=warn_off" desktop.pro
mingw32-make.exe release
cd ..

cd gdrdeamon
qmake "CONFIG+=warn_off" gdrdeamon.pro
mingw32-make.exe release
cd ..

cd gdrbridge
qmake "CONFIG+=warn_off" gdrbridge.pro
mingw32-make.exe release
cd ..

cd gdrexport
qmake "CONFIG+=warn_off" gdrexport.pro
mingw32-make.exe release
cd ..

cd scripts

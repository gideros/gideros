pushd \
call C:\Qt\Qt5.6.0\5.6\mingw49_32\bin\qtenv2.bat
popd

cd ..

cd texturepacker
mingw32-make.exe clean /f >nul 2>&1
qmake "CONFIG+=warn_off" texturepacker.pro
mingw32-make.exe clean /f >nul 2>&1
cd ..

cd fontcreator
mingw32-make.exe clean /f >nul 2>&1
qmake "CONFIG+=warn_off" fontcreator.pro
mingw32-make.exe clean /f >nul 2>&1
cd ..

cd ui
mingw32-make.exe clean /f >nul 2>&1
qmake "CONFIG+=warn_off" ui.pro
mingw32-make.exe clean /f >nul 2>&1
cd ..

cd player
mingw32-make.exe clean /f >nul 2>&1
qmake "CONFIG+=warn_off" player_qt5.pro
mingw32-make.exe clean /f >nul 2>&1
cd ..

cd desktop
mingw32-make.exe clean /f >nul 2>&1
qmake "CONFIG+=warn_off" desktop.pro
mingw32-make.exe clean /f >nul 2>&1
cd ..

cd gdrdeamon
mingw32-make.exe clean /f >nul 2>&1
qmake "CONFIG+=warn_off" gdrdeamon.pro
mingw32-make.exe clean /f >nul 2>&1
cd ..

cd gdrbridge
mingw32-make.exe clean /f >nul 2>&1
qmake "CONFIG+=warn_off" gdrbridge.pro
mingw32-make.exe clean /f >nul 2>&1
cd ..

cd gdrexport
mingw32-make.exe clean /f >nul 2>&1
qmake "CONFIG+=warn_off" gdrexport.pro
mingw32-make.exe clean /f >nul 2>&1
cd ..

cd scripts

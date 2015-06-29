cd ..

cd texturepacker
make clean > /dev/null 2>&1
$QT/bin/qmake "CONFIG+=warn_off" texturepacker.pro
make clean > /dev/null 2>&1
cd ..

cd fontcreator
make clean > /dev/null 2>&1
$QT/bin/qmake "CONFIG+=warn_off" fontcreator.pro
make clean > /dev/null 2>&1
cd ..

cd ui
make clean > /dev/null 2>&1
$QT/bin/qmake "CONFIG+=warn_off" ui.pro
make clean > /dev/null 2>&1
cd ..

cd player
make clean > /dev/null 2>&1
$QT/bin/qmake "CONFIG+=warn_off" player_qt5.pro
make clean > /dev/null 2>&1
cd ..

cd desktop
make clean > /dev/null 2>&1
$QT/bin/qmake "CONFIG+=warn_off" desktop.pro
make clean > /dev/null 2>&1
cd ..

cd gdrdeamon
make clean > /dev/null 2>&1
$QT/bin/qmake "CONFIG+=warn_off" gdrdeamon.pro
make clean > /dev/null 2>&1
cd ..

cd gdrbridge
make clean > /dev/null 2>&1
$QT/bin/qmake "CONFIG+=warn_off" gdrbridge.pro
make clean > /dev/null 2>&1
cd ..

cd gdrexport
make clean > /dev/null 2>&1
$QT/bin/qmake "CONFIG+=warn_off" gdrexport.pro
make clean > /dev/null 2>&1
cd ..



cd scripts

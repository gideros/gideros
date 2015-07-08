cd ..

cd texturepacker
$QT/bin/qmake "CONFIG+=warn_off" texturepacker.pro
make
cd ..

cd fontcreator
$QT/bin/qmake "CONFIG+=warn_off" fontcreator.pro
make
cd ..

cd ui
$QT/bin/qmake "CONFIG+=warn_off" ui.pro
make
cd ..

cd player
$QT/bin/qmake "CONFIG+=warn_off" player_qt5.pro
make
cd ..

cd desktop
$QT/bin/qmake "CONFIG+=warn_off" desktop.pro
make
cd ..

cd gdrdeamon
$QT/bin/qmake "CONFIG+=warn_off" gdrdeamon.pro
make
cd ..

cd gdrbridge
$QT/bin/qmake "CONFIG+=warn_off" gdrbridge.pro
make
cd ..

cd gdrexport
$QT/bin/qmake "CONFIG+=warn_off" gdrexport.pro
make
cd ..


cd scripts

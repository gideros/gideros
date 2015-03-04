cd ..

cd texturepacker
make clean
$QT/bin/qmake texturepacker.pro
make clean
cd ..

cd fontcreator
make clean
$QT/bin/qmake fontcreator.pro
make clean
cd ..

cd ui
make clean
$QT/bin/qmake ui.pro
make clean
cd ..

cd player
make clean
$QT/bin/qmake player_qt5.pro
make clean
cd ..

cd gdrdeamon
make clean
$QT/bin/qmake gdrdeamon.pro
make clean
cd ..

cd gdrbridge
make clean
$QT/bin/qmake gdrbridge.pro
make clean
cd ..

cd gdrexport
make clean
$QT/bin/qmake gdrexport.pro
make clean
cd ..



cd scripts

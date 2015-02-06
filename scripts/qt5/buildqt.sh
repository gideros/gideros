cd ..

cd texturepacker
$QT/bin/qmake texturepacker.pro
make
cd ..

cd fontcreator
$QT/bin/qmake fontcreator.pro
make
cd ..

cd ui
$QT/bin/qmake ui.pro
make
cd ..

cd player
$QT/bin/qmake player_qt5.pro
make
cd ..

cd licensemanager
$QT/bin/qmake licensemanager.pro
make
cd ..

cd gdrdeamon
$QT/bin/qmake gdrdeamon.pro
make
cd ..

cd gdrbridge
$QT/bin/qmake gdrbridge.pro
make
cd ..

cd gdrexport
$QT/bin/qmake gdrexport.pro
make
cd ..


cd scripts

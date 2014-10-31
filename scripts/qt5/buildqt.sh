QT=/Qt/Qt5.3.2/5.3

cd ..

cd texturepacker
$QT/clang_64/bin/qmake texturepacker.pro
make
cd ..

cd fontcreator
$QT/clang_64/bin/qmake fontcreator.pro
make
cd ..

cd ui
$QT/clang_64/bin/qmake ui.pro
make
cd ..

cd player
$QT/clang_64/bin/qmake player_qt5.pro
make
cd ..

cd licensemanager
$QT/clang_64/bin/qmake licensemanager.pro
make
cd ..

cd gdrdeamon
$QT/clang_64/bin/qmake gdrdeamon.pro
make
cd ..

cd gdrbridge
$QT/clang_64/bin/qmake gdrbridge.pro
make
cd ..

cd gdrexport
$QT/clang_64/bin/qmake gdrexport.pro
make
cd ..


cd scripts

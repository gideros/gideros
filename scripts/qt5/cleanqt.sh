QT=/Qt/Qt5.3.2/5.3

cd ..

cd texturepacker
make clean
$QT/clang_64/bin/qmake texturepacker.pro
make clean
cd ..

cd fontcreator
make clean
$QT/clang_64/bin/qmake fontcreator.pro
make clean
cd ..

cd ui
make clean
$QT/clang_64/bin/qmake ui.pro
make clean
cd ..

cd player
make clean
$QT/clang_64/bin/qmake player_qt5.pro
make clean
cd ..

cd licensemanager
make clean
$QT/clang_64/bin/qmake licensemanager.pro
make clean
cd ..

cd gdrdeamon
make clean
$QT/clang_64/bin/qmake gdrdeamon.pro
make clean
cd ..

cd gdrbridge
make clean
$QT/clang_64/bin/qmake gdrbridge.pro
make clean
cd ..

cd gdrexport
make clean
$QT/clang_64/bin/qmake gdrexport.pro
make clean
cd ..



cd scripts

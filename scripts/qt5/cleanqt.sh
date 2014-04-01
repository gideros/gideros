cd ..

cd texturepacker
make clean
/Qt/Qt5.2.1/5.2.1/clang_64/bin/qmake texturepacker.pro
make clean
cd ..

cd fontcreator
make clean
/Qt/Qt5.2.1/5.2.1/clang_64/bin/qmake fontcreator.pro
make clean
cd ..

cd ui
make clean
/Qt/Qt5.2.1/5.2.1/clang_64/bin/qmake ui.pro
make clean
cd ..

cd player
make clean
/Qt/Qt5.2.1/5.2.1/clang_64/bin/qmake player_qt5.pro
make clean
cd ..

cd licensemanager
make clean
/Qt/Qt5.2.1/5.2.1/clang_64/bin/qmake licensemanager.pro
make clean
cd ..

cd gdrdeamon
make clean
/Qt/Qt5.2.1/5.2.1/clang_64/bin/qmake gdrdeamon.pro
make clean
cd ..

cd gdrbridge
make clean
/Qt/Qt5.2.1/5.2.1/clang_64/bin/qmake gdrbridge.pro
make clean
cd ..

cd gdrexport
make clean
/Qt/Qt5.2.1/5.2.1/clang_64/bin/qmakem gdrexport.pro
make clean
cd ..



cd scripts

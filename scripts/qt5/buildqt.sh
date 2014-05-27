cd ..

cd texturepacker
/Qt/Qt5.3.0/5.3/clang_64/bin/qmake texturepacker.pro
make
cd ..

cd fontcreator
/Qt/Qt5.3.0/5.3/clang_64/bin/qmake fontcreator.pro
make
cd ..

cd ui
/Qt/Qt5.3.0/5.3/clang_64/bin/qmake ui.pro
make
cd ..

cd player
/Qt/Qt5.3.0/5.3/clang_64/bin/qmake player_qt5.pro
make
cd ..

cd licensemanager
/Qt/Qt5.3.0/5.3/clang_64/bin/qmake licensemanager.pro
make
cd ..

cd gdrdeamon
/Qt/Qt5.3.0/5.3/clang_64/bin/qmake gdrdeamon.pro
make
cd ..

cd gdrbridge
/Qt/Qt5.3.0/5.3/clang_64/bin/qmake gdrbridge.pro
make
cd ..

cd gdrexport
/Qt/Qt5.3.0/5.3/clang_64/bin/qmake gdrexport.pro
make
cd ..


cd scripts

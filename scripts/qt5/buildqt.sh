cd ..

cd texturepacker
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake texturepacker.pro
make
cd ..

cd fontcreator
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake fontcreator.pro
make
cd ..

cd ui
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake ui.pro
make
cd ..

cd player
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake player_qt5.pro
make
cd ..

cd licensemanager
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake licensemanager.pro
make
cd ..

cd gdrdeamon
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake gdrdeamon.pro
make
cd ..

cd gdrbridge
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake gdrbridge.pro
make
cd ..

cd gdrexport
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake gdrexport.pro
make
cd ..


cd scripts

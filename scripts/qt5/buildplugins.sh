cd ..
cd plugins

cd BitOp
cd source
$QT/bin/qmake bitop.pro
make clean
make
cd ../..

cd LuaSQLite3
cd source
$QT/bin/qmake lsqlite3.pro
make clean
make
cd ../..

cd LuaSocket
cd source
$QT/bin/qmake luasocket.pro
make clean
make
cd ../..

cd LPeg
cd source
$QT/bin/qmake lpeg.pro
make clean
make
cd ../..

cd LuaFileSystem
cd source
$QT/bin/qmake lfs.pro
make clean
make
cd ../..

cd Microphone
cd source
cd Desktop
$QT/bin/qmake microphone.pro
make clean
make
cd ../../..

cd JSON
cd source
$QT/bin/qmake json.pro
make clean
make
cd ../..

cd ..
cd scripts


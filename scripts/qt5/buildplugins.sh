QT=/Qt/Qt5.3.2/5.3

cd ..
cd plugins

cd BitOp
cd source
$QT/clang_64/bin/qmake bitop.pro
make clean
make
cd ../..

cd LuaSQLite3
cd source
$QT/clang_64/bin/qmake lsqlite3.pro
make clean
make
cd ../..

cd LuaSocket
cd source
$QT/clang_64/bin/qmake luasocket.pro
make clean
make
cd ../..

cd LPeg
cd source
$QT/clang_64/bin/qmake lpeg.pro
make clean
make
cd ../..

cd LuaFileSystem
cd source
$QT/clang_64/bin/qmake lfs.pro
make clean
make
cd ../..

cd Microphone
cd source
cd Desktop
$QT/clang_64/bin/qmake microphone.pro
make clean
make
cd ../../..

cd JSON
cd source
$QT/clang_64/bin/qmake json.pro
make clean
make
cd ../..

cd ..
cd scripts


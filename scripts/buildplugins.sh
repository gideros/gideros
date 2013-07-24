cd ..
cd plugins

cd BitOp
cd source
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake bitop.pro
make clean
make
cd ../..

cd LuaSQLite3
cd source
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake lsqlite3.pro
make clean
make
cd ../..

cd LuaSocket
cd source
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake luasocket.pro
make clean
make
cd ../..

cd LPeg
cd source
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake lpeg.pro
make clean
make
cd ../..

cd LuaFileSystem
cd source
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake lfs.pro
make clean
make
cd ../..

cd Microphone
cd source
/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/bin/qmake microphone.pro
make clean
make
cd ..\..



cd ..
cd scripts


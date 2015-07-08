# sudo rm /usr/lib/libpystring.1.dylib
# sudo rm /usr/lib/libgvfs.1.dylib
# sudo rm /usr/lib/libgid.1.dylib
# sudo rm /usr/lib/liblua.1.dylib
# sudo rm /usr/lib/libgideros.1.dylib

cd ..

cd libpystring
$QT/bin/qmake "CONFIG+=warn_off" libpystring.pro
make clean > /dev/null 2>&1
make
# sudo cp libpystring.1.dylib /usr/lib
cd ..

cd libgvfs
$QT/bin/qmake "CONFIG+=warn_off" libgvfs.pro
make clean > /dev/null 2>&1
make
# sudo cp libgvfs.1.dylib /usr/lib
cd ..

cd libgid
$QT/bin/qmake "CONFIG+=warn_off" libgid_qt5.pro
make clean > /dev/null 2>&1
make
# sudo cp libgid.1.dylib /usr/lib
cd ..

cd lua
$QT/bin/qmake "CONFIG+=warn_off" lua.pro
make clean > /dev/null 2>&1
make
# sudo cp liblua.1.dylib /usr/lib
cd ..

cd libgideros
$QT/bin/qmake "CONFIG+=warn_off" libgideros.pro
make clean > /dev/null 2>&1
make
# sudo cp libgideros.1.dylib /usr/lib
cd ..

# rm -rf Sdk
mkdir Sdk
cd Sdk
mkdir include
cp ../libgideros/gideros.h include
cp ../libgideros/gplugin.h include
cp ../libgideros/gproxy.h include
cp ../libgideros/greferenced.h include
cp ../libgideros/gexport.h include
cp ../libgvfs/gfile.h include
cp ../lua/src/lua.h include
cp ../lua/src/luaconf.h include
cp ../lua/src/lualib.h include
cp ../lua/src/lauxlib.h include
cp ../libgid/include/gglobal.h include
cp ../libgvfs/gpath.h include
cp ../libgid/include/glog.h include
cp ../libgid/include/gapplication.h include
cp ../libgid/include/gevent.h include


mkdir lib
mkdir lib/desktop
cp ../libgvfs/libgvfs.1.dylib lib/desktop
cp ../libgideros/libgideros.1.dylib lib/desktop
cp ../lua/liblua.1.dylib lib/desktop
cp ../libgid/libgid.1.dylib lib/desktop

cd lib/desktop
ln -s libgvfs.1.dylib libgvfs.dylib
ln -s libgideros.1.dylib libgideros.dylib
ln -s liblua.1.dylib liblua.dylib
ln -s libgid.1.dylib libgid.dylib
cd ../..

cd ..

cd scripts

BUILD_DIR=build

cd ..

mkdir $BUILD_DIR
rm -rf $BUILD_DIR/mac
mkdir $BUILD_DIR/mac

cp -R ui/Gideros\ Studio.app $BUILD_DIR/mac
cp -R player/Gideros\ Player.app $BUILD_DIR/mac
cp -R texturepacker/Gideros\ Texture\ Packer.app $BUILD_DIR/mac
cp -R fontcreator/Gideros\ Font\ Creator.app $BUILD_DIR/mac
cp -R licensemanager/Gideros\ License\ Manager.app $BUILD_DIR/mac

cp -R $BUILD_DIR/win/Documentation $BUILD_DIR/mac
cp -R $BUILD_DIR/win/Examples $BUILD_DIR/mac
cp -R $BUILD_DIR/win/Resources $BUILD_DIR/mac/Gideros\ Studio.app/Contents
cp -R $BUILD_DIR/win/Tools $BUILD_DIR/mac/Gideros\ Studio.app/Contents
cp -R $BUILD_DIR/win/Templates $BUILD_DIR/mac/Gideros\ Studio.app/Contents

cp gdrdeamon/gdrdeamon $BUILD_DIR/mac/Gideros\ Studio.app/Contents/Tools
cp gdrbridge/gdrbridge $BUILD_DIR/mac/Gideros\ Studio.app/Contents/Tools
cp gdrexport/gdrexport $BUILD_DIR/mac/Gideros\ Studio.app/Contents/Tools

sudo cp $QT/lib/libqscintilla2.11.dylib /usr/lib
sudo cp libgid/libgid.1.dylib /usr/lib
sudo cp libgvfs/libgvfs.1.dylib /usr/lib
sudo cp lua/liblua.1.dylib /usr/lib
sudo cp libgideros/libgideros.1.dylib /usr/lib
sudo cp libpystring/libpystring.1.dylib /usr/lib

$QT/bin/macdeployqt $BUILD_DIR/mac/Gideros\ Studio.app
$QT/bin/macdeployqt $BUILD_DIR/mac/Gideros\ Player.app
$QT/bin/macdeployqt $BUILD_DIR/mac/Gideros\ Texture\ Packer.app
$QT/bin/macdeployqt $BUILD_DIR/mac/Gideros\ Font\ Creator.app
$QT/bin/macdeployqt $BUILD_DIR/mac/Gideros\ License\ Manager.app

sudo rm /usr/lib/libqscintilla2.11.dylib
sudo rm /usr/lib/libgid.1.dylib
sudo rm /usr/lib/libgvfs.1.dylib
sudo rm /usr/lib/liblua.1.dylib
sudo rm /usr/lib/libgideros.1.dylib
sudo rm /usr/lib/libpystring.1.dylib

cp $BUILD_DIR/win/GiderosiOSPlayer.zip $BUILD_DIR/mac
cp $BUILD_DIR/win/GiderosAndroidPlayer.apk $BUILD_DIR/mac

cp -R Sdk $BUILD_DIR/mac
cp -R $BUILD_DIR/win/Sdk/lib/android $BUILD_DIR/mac/Sdk/lib

git archive -o $BUILD_DIR/tmp.tar HEAD:plugins
mkdir $BUILD_DIR/mac/All\ Plugins
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/mac/All\ Plugins

mkdir $BUILD_DIR/mac/All\ Plugins/BitOp/bin
mkdir $BUILD_DIR/mac/All\ Plugins/Facebook/bin
mkdir $BUILD_DIR/mac/All\ Plugins/Flurry/bin
mkdir $BUILD_DIR/mac/All\ Plugins/Game\ Kit/bin
mkdir $BUILD_DIR/mac/All\ Plugins/Google\ Billing/bin
mkdir $BUILD_DIR/mac/All\ Plugins/iAd/bin
mkdir $BUILD_DIR/mac/All\ Plugins/JSON/bin
mkdir $BUILD_DIR/mac/All\ Plugins/LPeg/bin
mkdir $BUILD_DIR/mac/All\ Plugins/LuaFileSystem/bin
mkdir $BUILD_DIR/mac/All\ Plugins/LuaSocket/bin
mkdir $BUILD_DIR/mac/All\ Plugins/LuaSQLite3/bin
mkdir $BUILD_DIR/mac/All\ Plugins/Microphone/bin
mkdir $BUILD_DIR/mac/All\ Plugins/Store\ Kit/bin

mkdir $BUILD_DIR/mac/All\ Plugins/BitOp/bin/Mac\ OS
cp plugins/BitOp/source/libbitop.dylib $BUILD_DIR/mac/All\ Plugins/BitOp/bin/Mac\ OS/bitop.dylib
mkdir $BUILD_DIR/mac/All\ Plugins/LuaSQLite3/bin/Mac\ OS
cp plugins/LuaSQLite3/source/liblsqlite3.dylib $BUILD_DIR/mac/All\ Plugins/LuaSQLite3/bin/Mac\ OS/lsqlite3.dylib
mkdir $BUILD_DIR/mac/All\ Plugins/LuaSocket/bin/Mac\ OS
cp plugins/LuaSocket/source/libluasocket.dylib $BUILD_DIR/mac/All\ Plugins/LuaSocket/bin/Mac\ OS/luasocket.dylib
mkdir $BUILD_DIR/mac/All\ Plugins/LPeg/bin/Mac\ OS
cp plugins/LPeg/source/liblpeg.dylib $BUILD_DIR/mac/All\ Plugins/LPeg/bin/Mac\ OS/lpeg.dylib
mkdir $BUILD_DIR/mac/All\ Plugins/LuaFileSystem/bin/Mac\ OS
cp plugins/LuaFileSystem/source/liblfs.dylib $BUILD_DIR/mac/All\ Plugins/LuaFileSystem/bin/Mac\ OS/lfs.dylib
mkdir $BUILD_DIR/mac/All\ Plugins/Microphone/bin/Mac\ OS
cp plugins/Microphone/source/Desktop/libmicrophone.dylib $BUILD_DIR/mac/All\ Plugins/Microphone/bin/Mac\ OS/microphone.dylib
mkdir $BUILD_DIR/mac/All\ Plugins/JSON/bin/Mac\ OS
cp plugins/JSON/source/libjson.dylib $BUILD_DIR/mac/All\ Plugins/JSON/bin/Mac\ OS/json.dylib


cp -R $BUILD_DIR/win/All\ Plugins/BitOp/bin/Android $BUILD_DIR/mac/All\ Plugins/BitOp/bin
cp -R $BUILD_DIR/win/All\ Plugins/LuaSocket/bin/Android $BUILD_DIR/mac/All\ Plugins/LuaSocket/bin
cp -R $BUILD_DIR/win/All\ Plugins/LPeg/bin/Android $BUILD_DIR/mac/All\ Plugins/LPeg/bin
cp -R $BUILD_DIR/win/All\ Plugins/LuaFileSystem/bin/Android $BUILD_DIR/mac/All\ Plugins/LuaFileSystem/bin
cp -R $BUILD_DIR/win/All\ Plugins/LuaSQLite3/bin/Android $BUILD_DIR/mac/All\ Plugins/LuaSQLite3/bin
cp -R $BUILD_DIR/win/All\ Plugins/Microphone/bin/Android $BUILD_DIR/mac/All\ Plugins/Microphone/bin
cp -R $BUILD_DIR/win/All\ Plugins/JSON/bin/Android $BUILD_DIR/mac/All\ Plugins/JSON/bin
cp -R $BUILD_DIR/win/All\ Plugins/Facebook/bin/Android $BUILD_DIR/mac/All\ Plugins/Facebook/bin
cp -R $BUILD_DIR/win/All\ Plugins/Flurry/bin/Android $BUILD_DIR/mac/All\ Plugins/Flurry/bin
cp -R $BUILD_DIR/win/All\ Plugins/Google\ Billing/bin/Android $BUILD_DIR/mac/All\ Plugins/Google\ Billing/bin

mkdir $BUILD_DIR/mac/Plugins
cp plugins/LuaSQLite3/source/liblsqlite3.dylib $BUILD_DIR/mac/Plugins/lsqlite3.dylib
cp plugins/LuaSocket/source/libluasocket.dylib $BUILD_DIR/mac/Plugins/luasocket.dylib
cp plugins/LuaFileSystem/source/liblfs.dylib $BUILD_DIR/mac/Plugins/lfs.dylib
cp plugins/Microphone/source/Desktop/libmicrophone.dylib $BUILD_DIR/mac/Plugins/microphone.dylib
cp plugins/BitOp/source/libbitop.dylib $BUILD_DIR/mac/Plugins/bitop.dylib
cp plugins/JSON/source/libjson.dylib $BUILD_DIR/mac/Plugins/json.dylib

cd scripts











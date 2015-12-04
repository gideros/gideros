BUILD_DIR=build

cd ..

mkdir $BUILD_DIR
rm -rf $BUILD_DIR/mac
mkdir $BUILD_DIR/mac

cp -R ui/Gideros\ Studio.app $BUILD_DIR/mac
cp -R player/Gideros\ Player.app $BUILD_DIR/mac
cp -R texturepacker/Gideros\ Texture\ Packer.app $BUILD_DIR/mac
cp -R fontcreator/Gideros\ Font\ Creator.app $BUILD_DIR/mac

cp -R $BUILD_DIR/win/Documentation $BUILD_DIR/mac
cp -R $BUILD_DIR/win/Examples $BUILD_DIR/mac
cp -R $BUILD_DIR/win/Resources $BUILD_DIR/mac/Gideros\ Studio.app/Contents
cp -R $BUILD_DIR/win/Tools $BUILD_DIR/mac/Gideros\ Studio.app/Contents
cp -R $BUILD_DIR/win/Templates $BUILD_DIR/mac/Gideros\ Studio.app/Contents
mkdir $BUILD_DIR/mac/Gideros\ Studio.app/Contents/Templates/Qt/MacOSXDesktopTemplate
cp -R desktop/MacOSXDesktopTemplate.app $BUILD_DIR/mac/Gideros\ Studio.app/Contents/Templates/Qt/MacOSXDesktopTemplate
cp -R desktop/Entitlements.plist $BUILD_DIR/mac/Gideros\ Studio.app/Contents/Templates/Qt/MacOSXDesktopTemplate

install_name_tool -add_rpath @executable_path/../Frameworks gdrdeamon/gdrdeamon
install_name_tool -add_rpath @executable_path/../Frameworks gdrbridge/gdrbridge
install_name_tool -add_rpath @executable_path/../Frameworks gdrexport/gdrexport

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
$QT/bin/macdeployqt $BUILD_DIR/mac/Gideros\ Studio.app/Contents/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app

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

cd plugins
for d in *; do
cd $d/source
mkdir ../../../$BUILD_DIR/mac/All\ Plugins/$d
mkdir ../../../$BUILD_DIR/mac/All\ Plugins/$d/bin
mkdir ../../../$BUILD_DIR/mac/Plugins
if [ -f $d.pro ]; then
mkdir ../../../$BUILD_DIR/mac/All\ Plugins/$d/bin/Mac\ OS
cp lib$d.dylib ../../../$BUILD_DIR/mac/All\ Plugins/$d/bin/Mac\ OS/$d.dylib
cp lib$d.dylib ../../../$BUILD_DIR/mac/Plugins/$d.dylib
cp lib$d.dylib ../../../$BUILD_DIR/mac/Gideros\ Studio.app/Contents/Templates/Qt/MacOSXDesktopTemplate/MacOSXDesktopTemplate.app/Contents/PlugIns/$d.dylib
fi
if [ -d Android ] || [ -d jni ] ; then
if [ -d Android ]; then
cd Android
cp -R ../../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android ../../../../$BUILD_DIR/mac/All\ Plugins/$d/bin
cd ..
else
cp -R ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android ../../../$BUILD_DIR/mac/All\ Plugins/$d/bin
fi
fi
cd ..
cd ..
done
cd ..

cp -R $BUILD_DIR/mac/Gideros\ Studio.app/Contents/Templates/Qt/MacOSXDesktopTemplate $BUILD_DIR/win/Templates/Qt

cd scripts











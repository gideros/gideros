BUILD_DIR=build

cd ..

mkdir $BUILD_DIR
rm -rf $BUILD_DIR/win
mkdir $BUILD_DIR/win

cp libgid/release/gid.dll $BUILD_DIR/win
cp libgvfs/release/gvfs.dll $BUILD_DIR/win
cp lua/release/lua.dll $BUILD_DIR/win
cp libgideros/release/gideros.dll $BUILD_DIR/win
cp libpystring/release/pystring.dll $BUILD_DIR/win

cp ui/release/GiderosStudio.exe $BUILD_DIR/win
cp player/release/GiderosPlayer.exe $BUILD_DIR/win
cp texturepacker/release/GiderosTexturePacker.exe $BUILD_DIR/win
cp fontcreator/release/GiderosFontCreator.exe $BUILD_DIR/win

git archive -o $BUILD_DIR/tmp.tar HEAD:ui/Resources
mkdir $BUILD_DIR/win/Resources
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/Resources

git archive -o $BUILD_DIR/tmp.tar HEAD:ui/Tools
mkdir $BUILD_DIR/win/Tools
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/Tools

cp $QT_WIN/bin/icudt$QT_DLL.dll $BUILD_DIR/win
cp $QT_WIN/bin/icuin$QT_DLL.dll $BUILD_DIR/win
cp $QT_WIN/bin/icuuc$QT_DLL.dll $BUILD_DIR/win
cp $QT_WIN/bin/libgcc_s_dw2-1.dll $BUILD_DIR/win
cp $QT_WIN/bin/libstdc++-6.dll $BUILD_DIR/win
cp $QT_WIN/bin/libwinpthread-1.dll $BUILD_DIR/win
cp $QT_WIN/bin/Qt5Core.dll $BUILD_DIR/win
cp $QT_WIN/bin/Qt5Gui.dll $BUILD_DIR/win
cp $QT_WIN/bin/Qt5Network.dll $BUILD_DIR/win
cp $QT_WIN/bin/Qt5OpenGL.dll $BUILD_DIR/win
cp $QT_WIN/bin/Qt5PrintSupport.dll $BUILD_DIR/win
cp $QT_WIN/bin/Qt5Widgets.dll $BUILD_DIR/win
cp $QT_WIN/bin/Qt5Xml.dll $BUILD_DIR/win
cp $QT_WIN/bin/Qt5Multimedia.dll $BUILD_DIR/win
cp $QT_WIN/bin/Qt5MultimediaQuick_p.dll $BUILD_DIR/win
cp $QT_WIN/bin/Qt5MultimediaWidgets.dll $BUILD_DIR/win

mkdir $BUILD_DIR/win/imageformats
cp $QT_WIN/plugins/imageformats/qjpeg.dll $BUILD_DIR/win/imageformats

mkdir $BUILD_DIR/win/platforms
cp $QT_WIN/plugins/platforms/qminimal.dll $BUILD_DIR/win/platforms
cp $QT_WIN/plugins/platforms/qoffscreen.dll $BUILD_DIR/win/platforms
cp $QT_WIN/plugins/platforms/qwindows.dll $BUILD_DIR/win/platforms

cp $QT_WIN/lib/qscintilla2.dll $BUILD_DIR/win

cp libgid/external/glew-1.10.0/lib/mingw48_32/glew32.dll $BUILD_DIR/win
cp libgid/external/openal-soft-1.13/build/mingw48_32/OpenAL32.dll $BUILD_DIR/win

cp $BUILD_DIR/win/icudt$QT_DLL.dll $BUILD_DIR/win/Tools
cp $BUILD_DIR/win/icuin$QT_DLL.dll $BUILD_DIR/win/Tools
cp $BUILD_DIR/win/icuuc$QT_DLL.dll $BUILD_DIR/win/Tools
cp $BUILD_DIR/win/libgcc_s_dw2-1.dll $BUILD_DIR/win/Tools
cp $BUILD_DIR/win/libstdc++-6.dll $BUILD_DIR/win/Tools
cp $BUILD_DIR/win/libwinpthread-1.dll $BUILD_DIR/win/Tools
cp $BUILD_DIR/win/Qt5Core.dll $BUILD_DIR/win/Tools
cp $BUILD_DIR/win/Qt5Network.dll $BUILD_DIR/win/Tools
cp $BUILD_DIR/win/Qt5Xml.dll $BUILD_DIR/win/Tools

mkdir $BUILD_DIR/win/Templates

git archive -o $BUILD_DIR/tmp.tar HEAD:ui/Templates/Eclipse
mkdir $BUILD_DIR/win/Templates/Eclipse
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/Templates/Eclipse

git archive -o $BUILD_DIR/tmp.tar HEAD:ui/Templates/AndroidStudio
mkdir $BUILD_DIR/win/Templates/AndroidStudio
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/Templates/AndroidStudio

git archive -o $BUILD_DIR/tmp.tar HEAD:ui/Templates/Xcode4
mkdir $BUILD_DIR/win/Templates/Xcode4
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/Templates/Xcode4

git archive -o $BUILD_DIR/tmp.tar HEAD:ui/Templates/APK
mkdir $BUILD_DIR/win/Templates/APK
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/Templates/APK
cp $BUILD_DIR/../ui/Templates/APK.gexport $BUILD_DIR/win/Templates

mkdir $BUILD_DIR/win/Templates/Eclipse/Android\ Template/assets
mkdir $BUILD_DIR/win/Templates/Eclipse/Android\ Template/gen
mkdir $BUILD_DIR/win/Templates/Eclipse/Android\ Template/res/layout
mkdir $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/libs
mkdir $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/assets
mkdir $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs
mkdir $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/iOS\ Template/assets
mkdir $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/AppleTV/assets

mkdir $BUILD_DIR/win/Templates/Qt
mkdir $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp desktop/release/WindowsDesktopTemplate.exe $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/icudt$QT_DLL.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/icuin$QT_DLL.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/icuuc$QT_DLL.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/libgcc_s_dw2-1.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/libstdc++-6.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/libwinpthread-1.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/Qt5Core.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/Qt5Gui.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/Qt5Network.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/Qt5OpenGL.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/Qt5PrintSupport.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/Qt5Widgets.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/Qt5Xml.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/Qt5Multimedia.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/Qt5MultimediaQuick_p.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/Qt5MultimediaWidgets.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
mkdir $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate/platforms
cp $BUILD_DIR/win/platforms/qminimal.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate/platforms
cp $BUILD_DIR/win/platforms/qoffscreen.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate/platforms
cp $BUILD_DIR/win/platforms/qwindows.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate/platforms
cp $BUILD_DIR/win/gid.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/gvfs.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/lua.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/gideros.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp $BUILD_DIR/win/pystring.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp libgid/external/glew-1.10.0/lib/mingw48_32/glew32.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
cp libgid/external/openal-soft-1.13/build/mingw48_32/OpenAL32.dll $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate
mkdir $BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate/Plugins


git archive -o $BUILD_DIR/tmp.tar HEAD:samplecode
mkdir $BUILD_DIR/win/Examples
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/Examples

git archive -o $BUILD_DIR/tmp.tar HEAD:ios/GiderosiOSPlayer
mkdir $BUILD_DIR/win/GiderosiOSPlayer
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/GiderosiOSPlayer

cp gdrdeamon/release/gdrdeamon.exe $BUILD_DIR/win/Tools
cp gdrbridge/release/gdrbridge.exe $BUILD_DIR/win/Tools
cp gdrexport/release/gdrexport.exe $BUILD_DIR/win/Tools

cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/giderosapi.h  $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/iOS\ Template
cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/*.a           $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/iOS\ Template
cp ui/Templates/Xcode4/iOS\ Template/AppleTV/giderosapi.h  $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/AppleTV
cp ui/Templates/Xcode4/iOS\ Template/AppleTV/*.a           $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/AppleTV
cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/giderosapi.h  $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer
cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/*.a           $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer

mkdir $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp Sdk/include/*.h $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins

cp plugins/gamekit/source/iOS/gamekit.mm $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/storekit/source/iOS/storekit.mm $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/mficontroller/source/iOS/mficontroller.mm $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/iad/source/iOS/iad.mm $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/lsqlite3/source/lsqlite3.c $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/lsqlite3/source/lsqlite3_stub.cpp  $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaSocket/source/luasocket_stub.cpp $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/Plugins/libluasocket.a $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/lfs/source/lfs.h $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/lfs/source/lfs.c $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/lfs/source/lfs_stub.cpp $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/BitOp/source/bit.c $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/BitOp/source/bit_stub.cpp $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/JSON/source/fpconv.c $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/JSON/source/fpconv.h $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/JSON/source/strbuf.c $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/JSON/source/strbuf.h $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/JSON/source/lua_cjson.c $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/JSON/source/lua_cjson_stub.cpp $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins

mkdir $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/iOS\ Template/Plugins
cp $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins/* $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/iOS\ Template/Plugins
mkdir $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/AppleTV/Plugins
cp $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins/* $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/AppleTV/Plugins
cp ui/Templates/Xcode4/iOS\ Template/AppleTV/Plugins/libluasocket.a $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/AppleTV/Plugins


cp android/GiderosAndroidPlayer/gideros.jar $BUILD_DIR/win/Templates/Eclipse/Android\ Template
cp android/GiderosAndroidPlayer/gideros.jar $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/libs
mkdir $BUILD_DIR/win/Templates/Eclipse/Android\ Template/jni
cp android/lib/jni/Application.mk $BUILD_DIR/win/Templates/Eclipse/Android\ Template/jni
cp -R android/build/libs $BUILD_DIR/win/Templates/Eclipse/Android\ Template
cp -R android/build/libs/. $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/

cp plugins/LuaSocket/source/libs/armeabi/libluasocket.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/LuaSocket/source/libs/armeabi-v7a/libluasocket.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/LuaSocket/source/libs/x86/libluasocket.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/lfs/source/libs/armeabi/liblfs.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/lfs/source/libs/armeabi-v7a/liblfs.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/lfs/source/libs/x86/liblfs.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/lsqlite3/source/libs/armeabi/liblsqlite3.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/lsqlite3/source/libs/armeabi-v7a/liblsqlite3.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/lsqlite3/source/libs/x86/liblsqlite3.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/BitOp/source/libs/armeabi/libbitop.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/BitOp/source/libs/armeabi-v7a/libbitop.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/BitOp/source/libs/x86/libbitop.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/JSON/source/libs/armeabi/libjson.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/JSON/source/libs/armeabi-v7a/libjson.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/JSON/source/libs/x86/libjson.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86


cp plugins/LuaSocket/source/libs/armeabi/libluasocket.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/armeabi
cp plugins/LuaSocket/source/libs/armeabi-v7a/libluasocket.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/armeabi-v7a
cp plugins/LuaSocket/source/libs/x86/libluasocket.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/x86

cp plugins/lfs/source/libs/armeabi/liblfs.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/armeabi
cp plugins/lfs/source/libs/armeabi-v7a/liblfs.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/armeabi-v7a
cp plugins/lfs/source/libs/x86/liblfs.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/x86

cp plugins/lsqlite3/source/libs/armeabi/liblsqlite3.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/armeabi
cp plugins/lsqlite3/source/libs/armeabi-v7a/liblsqlite3.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/armeabi-v7a
cp plugins/lsqlite3/source/libs/x86/liblsqlite3.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/x86

cp plugins/BitOp/source/libs/armeabi/libbitop.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/armeabi
cp plugins/BitOp/source/libs/armeabi-v7a/libbitop.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/armeabi-v7a
cp plugins/BitOp/source/libs/x86/libbitop.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/x86

cp plugins/JSON/source/libs/armeabi/libjson.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/armeabi
cp plugins/JSON/source/libs/armeabi-v7a/libjson.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/armeabi-v7a
cp plugins/JSON/source/libs/x86/libjson.so $BUILD_DIR/win/Templates/AndroidStudio/Android\ Template/app/src/main/jniLibs/x86


rm -rf android/GiderosAndroidPlayer/libs
cp -R $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs android/GiderosAndroidPlayer

cd android/GiderosAndroidPlayer
ant debug
mv bin/GiderosAndroidPlayer-debug.apk ../../$BUILD_DIR/win/GiderosAndroidPlayer.apk
cd ../..

cd $BUILD_DIR/win
zip -r GiderosiOSPlayer.zip GiderosiOSPlayer
rm -rf GiderosiOSPlayer
cd ../..


git archive -o $BUILD_DIR/tmp.tar HEAD:doc
mkdir $BUILD_DIR/win/Documentation
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/Documentation

cp licenses.txt $BUILD_DIR/win

cp -R Sdk $BUILD_DIR/win


git archive -o $BUILD_DIR/tmp.tar HEAD:plugins
mkdir $BUILD_DIR/win/All\ Plugins
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/All\ Plugins

mkdir $BUILD_DIR/win/Plugins

cd plugins
for d in *; do
cd $d/source
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin
cp -r ../examples ../../../$BUILD_DIR/win/All\ Plugins/$d
if [ -f $d.pro ]; then
cp release/$d.dll ../../../$BUILD_DIR/win/Plugins
cp release/$d.dll ../../../$BUILD_DIR/win/Templates/Qt/WindowsDesktopTemplate/Plugins
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Windows
cp release/$d.dll ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Windows
fi
if [ -d Android ] || [ -d jni ] ; then
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/libs
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/libs/armeabi
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/libs/armeabi-v7a
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/libs/x86
if [ -d Android ] ; then
cd Android
cp libs/armeabi/lib$d.so ../../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/libs/armeabi
cp libs/armeabi-v7a/lib$d.so ../../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/libs/armeabi-v7a
cp libs/x86/lib$d.so ../../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/libs/x86
cp -r src ../../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android
cp -r res ../../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android
cp -r assets ../../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android
cd ..
else
cp libs/armeabi/lib$d.so ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/libs/armeabi
cp libs/armeabi-v7a/lib$d.so ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/libs/armeabi-v7a
cp libs/x86/lib$d.so ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/libs/x86
fi
fi
cd ..
cd ..
done
cd ..

cd scripts


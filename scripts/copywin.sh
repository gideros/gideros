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

cp $QT_WIN/5.3/mingw482_32/bin/icudt52.dll $BUILD_DIR/win
cp $QT_WIN/5.3/mingw482_32/bin/icuin52.dll $BUILD_DIR/win
cp $QT_WIN/5.3/mingw482_32/bin/icuuc52.dll $BUILD_DIR/win
cp $QT_WIN/5.3/mingw482_32/bin/libgcc_s_dw2-1.dll $BUILD_DIR/win
cp $QT_WIN/5.3/mingw482_32/bin/libstdc++-6.dll $BUILD_DIR/win
cp $QT_WIN/5.3/mingw482_32/bin/libwinpthread-1.dll $BUILD_DIR/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Core.dll $BUILD_DIR/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Gui.dll $BUILD_DIR/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Network.dll $BUILD_DIR/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5OpenGL.dll $BUILD_DIR/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5PrintSupport.dll $BUILD_DIR/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Widgets.dll $BUILD_DIR/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Xml.dll $BUILD_DIR/win

mkdir $BUILD_DIR/win/imageformats
cp $QT_WIN/5.3/mingw482_32/plugins/imageformats/qjpeg.dll $BUILD_DIR/win/imageformats

mkdir $BUILD_DIR/win/platforms
cp $QT_WIN/5.3/mingw482_32/plugins/platforms/qminimal.dll $BUILD_DIR/win/platforms
cp $QT_WIN/5.3/mingw482_32/plugins/platforms/qoffscreen.dll $BUILD_DIR/win/platforms
cp $QT_WIN/5.3/mingw482_32/plugins/platforms/qwindows.dll $BUILD_DIR/win/platforms

cp $QT_WIN/5.3/mingw482_32/lib/qscintilla2.dll $BUILD_DIR/win

cp libgid/external/glew-1.10.0/lib/mingw48_32/glew32.dll $BUILD_DIR/win
cp libgid/external/openal-soft-1.13/build/mingw48_32/OpenAL32.dll $BUILD_DIR/win

cp $QT_WIN/5.3/mingw482_32/bin/icudt52.dll $BUILD_DIR/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/icuin52.dll $BUILD_DIR/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/icuuc52.dll $BUILD_DIR/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/libgcc_s_dw2-1.dll $BUILD_DIR/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/libstdc++-6.dll $BUILD_DIR/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/libwinpthread-1.dll $BUILD_DIR/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Core.dll $BUILD_DIR/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Network.dll $BUILD_DIR/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Xml.dll $BUILD_DIR/win/Tools

mkdir $BUILD_DIR/win/Templates

git archive -o $BUILD_DIR/tmp.tar HEAD:ui/Templates/Eclipse
mkdir $BUILD_DIR/win/Templates/Eclipse
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/Templates/Eclipse

git archive -o $BUILD_DIR/tmp.tar HEAD:ui/Templates/Xcode4
mkdir $BUILD_DIR/win/Templates/Xcode4
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/Templates/Xcode4

mkdir $BUILD_DIR/win/Templates/Eclipse/Android\ Template/assets
mkdir $BUILD_DIR/win/Templates/Eclipse/Android\ Template/gen
mkdir $BUILD_DIR/win/Templates/Eclipse/Android\ Template/res/layout
mkdir $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/iOS\ Template/assets

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
cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/giderosapi.h  $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer
cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/*.a           $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer

mkdir $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp Sdk/include/*.h $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/Game\ Kit/source/iOS/gamekit.mm $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/Store\ Kit/source/iOS/storekit.mm $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/iAd/source/iOS/iad.mm $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaSQLite3/source/lsqlite3.c $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaSQLite3/source/lsqlite3_stub.cpp  $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaSocket/source/luasocket_stub.cpp $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/Plugins/libluasocket.a $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaFileSystem/source/lfs.h $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaFileSystem/source/lfs.c $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaFileSystem/source/lfs_stub.cpp $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
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


cp android/GiderosAndroidPlayer/gideros.jar $BUILD_DIR/win/Templates/Eclipse/Android\ Template
mkdir $BUILD_DIR/win/Templates/Eclipse/Android\ Template/jni
cp android/lib/jni/Application.mk $BUILD_DIR/win/Templates/Eclipse/Android\ Template/jni
cp -R android/build/libs $BUILD_DIR/win/Templates/Eclipse/Android\ Template


cp plugins/LuaSocket/source/libs/armeabi/libluasocket.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/LuaSocket/source/libs/armeabi-v7a/libluasocket.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/LuaSocket/source/libs/x86/libluasocket.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/LuaFileSystem/source/libs/armeabi/liblfs.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/LuaFileSystem/source/libs/armeabi-v7a/liblfs.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/LuaFileSystem/source/libs/x86/liblfs.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/Google\ Billing/source/Android/libs/armeabi/libggooglebilling.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/Google\ Billing/source/Android/libs/armeabi-v7a/libggooglebilling.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/Google\ Billing/source/Android/libs/x86/libggooglebilling.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/LuaSQLite3/source/libs/armeabi/liblsqlite3.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/LuaSQLite3/source/libs/armeabi-v7a/liblsqlite3.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/LuaSQLite3/source/libs/x86/liblsqlite3.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/BitOp/source/libs/armeabi/libbitop.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/BitOp/source/libs/armeabi-v7a/libbitop.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/BitOp/source/libs/x86/libbitop.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/JSON/source/libs/armeabi/libjson.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/JSON/source/libs/armeabi-v7a/libjson.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/JSON/source/libs/x86/libjson.so $BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86

git archive -o $BUILD_DIR/tmp.tar HEAD:plugins/Google\ Billing/source/Android/com
tar xf $BUILD_DIR/tmp.tar -C $BUILD_DIR/win/Templates/Eclipse/Android\ Template/src/com
tar xf $BUILD_DIR/tmp.tar -C android/GiderosAndroidPlayer/src/com

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

mkdir $BUILD_DIR/win/All\ Plugins/BitOp/bin
mkdir $BUILD_DIR/win/All\ Plugins/Facebook/bin
mkdir $BUILD_DIR/win/All\ Plugins/Flurry/bin
mkdir $BUILD_DIR/win/All\ Plugins/Game\ Kit/bin
mkdir $BUILD_DIR/win/All\ Plugins/Google\ Billing/bin
mkdir $BUILD_DIR/win/All\ Plugins/iAd/bin
mkdir $BUILD_DIR/win/All\ Plugins/JSON/bin
mkdir $BUILD_DIR/win/All\ Plugins/LPeg/bin
mkdir $BUILD_DIR/win/All\ Plugins/LuaFileSystem/bin
mkdir $BUILD_DIR/win/All\ Plugins/LuaSocket/bin
mkdir $BUILD_DIR/win/All\ Plugins/LuaSQLite3/bin
mkdir $BUILD_DIR/win/All\ Plugins/Microphone/bin
mkdir $BUILD_DIR/win/All\ Plugins/Store\ Kit/bin
mkdir $BUILD_DIR/win/All\ Plugins/BitOp/bin/Windows
cp plugins/BitOp/source/release/bitop.dll $BUILD_DIR/win/All\ Plugins/BitOp/bin/Windows
mkdir $BUILD_DIR/win/All\ Plugins/LuaSQLite3/bin/Windows
cp plugins/LuaSQLite3/source/release/lsqlite3.dll $BUILD_DIR/win/All\ Plugins/LuaSQLite3/bin/Windows
mkdir $BUILD_DIR/win/All\ Plugins/LuaSocket/bin/Windows
cp plugins/LuaSocket/source/release/luasocket.dll $BUILD_DIR/win/All\ Plugins/LuaSocket/bin/Windows
mkdir $BUILD_DIR/win/All\ Plugins/LuaFileSystem/bin/Windows
cp plugins/LuaFileSystem/source/release/lfs.dll $BUILD_DIR/win/All\ Plugins/LuaFileSystem/bin/Windows
mkdir $BUILD_DIR/win/All\ Plugins/LPeg/bin/Windows
cp plugins/LPeg/source/release/lpeg.dll $BUILD_DIR/win/All\ Plugins/LPeg/bin/Windows
mkdir $BUILD_DIR/win/All\ Plugins/Microphone/bin/Windows
cp plugins/Microphone/source/Desktop/release/microphone.dll $BUILD_DIR/win/All\ Plugins/Microphone/bin/Windows
mkdir $BUILD_DIR/win/All\ Plugins/JSON/bin/Windows
cp plugins/JSON/source/release/json.dll $BUILD_DIR/win/All\ Plugins/JSON/bin/Windows


mkdir $BUILD_DIR/win/All\ Plugins/BitOp/bin/Android
mkdir $BUILD_DIR/win/All\ Plugins/BitOp/bin/Android/armeabi
mkdir $BUILD_DIR/win/All\ Plugins/BitOp/bin/Android/armeabi-v7a
mkdir $BUILD_DIR/win/All\ Plugins/BitOp/bin/Android/x86
cp plugins/BitOp/source/libs/armeabi/libbitop.so $BUILD_DIR/win/All\ Plugins/BitOp/bin/Android/armeabi
cp plugins/BitOp/source/libs/armeabi-v7a/libbitop.so $BUILD_DIR/win/All\ Plugins/BitOp/bin/Android/armeabi-v7a
cp plugins/BitOp/source/libs/x86/libbitop.so $BUILD_DIR/win/All\ Plugins/BitOp/bin/Android/x86

mkdir $BUILD_DIR/win/All\ Plugins/LuaSocket/bin/Android
mkdir $BUILD_DIR/win/All\ Plugins/LuaSocket/bin/Android/armeabi
mkdir $BUILD_DIR/win/All\ Plugins/LuaSocket/bin/Android/armeabi-v7a
mkdir $BUILD_DIR/win/All\ Plugins/LuaSocket/bin/Android/x86
cp plugins/LuaSocket/source/libs/armeabi/libluasocket.so $BUILD_DIR/win/All\ Plugins/LuaSocket/bin/Android/armeabi
cp plugins/LuaSocket/source/libs/armeabi-v7a/libluasocket.so $BUILD_DIR/win/All\ Plugins/LuaSocket/bin/Android/armeabi-v7a
cp plugins/LuaSocket/source/libs/x86/libluasocket.so $BUILD_DIR/win/All\ Plugins/LuaSocket/bin/Android/x86

mkdir $BUILD_DIR/win/All\ Plugins/LPeg/bin/Android
mkdir $BUILD_DIR/win/All\ Plugins/LPeg/bin/Android/armeabi
mkdir $BUILD_DIR/win/All\ Plugins/LPeg/bin/Android/armeabi-v7a
mkdir $BUILD_DIR/win/All\ Plugins/LPeg/bin/Android/x86
cp plugins/LPeg/source/libs/armeabi/liblpeg.so $BUILD_DIR/win/All\ Plugins/LPeg/bin/Android/armeabi
cp plugins/LPeg/source/libs/armeabi-v7a/liblpeg.so $BUILD_DIR/win/All\ Plugins/LPeg/bin/Android/armeabi-v7a
cp plugins/LPeg/source/libs/x86/liblpeg.so $BUILD_DIR/win/All\ Plugins/LPeg/bin/Android/x86

mkdir $BUILD_DIR/win/All\ Plugins/LuaFileSystem/bin/Android
mkdir $BUILD_DIR/win/All\ Plugins/LuaFileSystem/bin/Android/armeabi
mkdir $BUILD_DIR/win/All\ Plugins/LuaFileSystem/bin/Android/armeabi-v7a
mkdir $BUILD_DIR/win/All\ Plugins/LuaFileSystem/bin/Android/x86
cp plugins/LuaFileSystem/source/libs/armeabi/liblfs.so $BUILD_DIR/win/All\ Plugins/LuaFileSystem/bin/Android/armeabi
cp plugins/LuaFileSystem/source/libs/armeabi-v7a/liblfs.so $BUILD_DIR/win/All\ Plugins/LuaFileSystem/bin/Android/armeabi-v7a
cp plugins/LuaFileSystem/source/libs/x86/liblfs.so $BUILD_DIR/win/All\ Plugins/LuaFileSystem/bin/Android/x86

mkdir $BUILD_DIR/win/All\ Plugins/LuaSQLite3/bin/Android
mkdir $BUILD_DIR/win/All\ Plugins/LuaSQLite3/bin/Android/armeabi
mkdir $BUILD_DIR/win/All\ Plugins/LuaSQLite3/bin/Android/armeabi-v7a
mkdir $BUILD_DIR/win/All\ Plugins/LuaSQLite3/bin/Android/x86
cp plugins/LuaSQLite3/source/libs/armeabi/liblsqlite3.so $BUILD_DIR/win/All\ Plugins/LuaSQLite3/bin/Android/armeabi
cp plugins/LuaSQLite3/source/libs/armeabi-v7a/liblsqlite3.so $BUILD_DIR/win/All\ Plugins/LuaSQLite3/bin/Android/armeabi-v7a
cp plugins/LuaSQLite3/source/libs/x86/liblsqlite3.so $BUILD_DIR/win/All\ Plugins/LuaSQLite3/bin/Android/x86

mkdir $BUILD_DIR/win/All\ Plugins/Microphone/bin/Android
mkdir $BUILD_DIR/win/All\ Plugins/Microphone/bin/Android/armeabi
mkdir $BUILD_DIR/win/All\ Plugins/Microphone/bin/Android/armeabi-v7a
mkdir $BUILD_DIR/win/All\ Plugins/Microphone/bin/Android/x86
cp plugins/Microphone/source/Android/libs/armeabi/libmicrophone.so $BUILD_DIR/win/All\ Plugins/Microphone/bin/Android/armeabi
cp plugins/Microphone/source/Android/libs/armeabi-v7a/libmicrophone.so $BUILD_DIR/win/All\ Plugins/Microphone/bin/Android/armeabi-v7a
cp plugins/Microphone/source/Android/libs/x86/libmicrophone.so $BUILD_DIR/win/All\ Plugins/Microphone/bin/Android/x86

mkdir $BUILD_DIR/win/All\ Plugins/JSON/bin/Android
mkdir $BUILD_DIR/win/All\ Plugins/JSON/bin/Android/armeabi
mkdir $BUILD_DIR/win/All\ Plugins/JSON/bin/Android/armeabi-v7a
mkdir $BUILD_DIR/win/All\ Plugins/JSON/bin/Android/x86
cp plugins/JSON/source/libs/armeabi/libjson.so $BUILD_DIR/win/All\ Plugins/JSON/bin/Android/armeabi
cp plugins/JSON/source/libs/armeabi-v7a/libjson.so $BUILD_DIR/win/All\ Plugins/JSON/bin/Android/armeabi-v7a
cp plugins/JSON/source/libs/x86/libjson.so $BUILD_DIR/win/All\ Plugins/JSON/bin/Android/x86

mkdir $BUILD_DIR/win/All\ Plugins/Flurry/bin/Android
mkdir $BUILD_DIR/win/All\ Plugins/Flurry/bin/Android/armeabi
mkdir $BUILD_DIR/win/All\ Plugins/Flurry/bin/Android/armeabi-v7a
mkdir $BUILD_DIR/win/All\ Plugins/Flurry/bin/Android/x86
cp plugins/Flurry/source/Android/libs/armeabi/libflurry.so $BUILD_DIR/win/All\ Plugins/Flurry/bin/Android/armeabi
cp plugins/Flurry/source/Android/libs/armeabi-v7a/libflurry.so $BUILD_DIR/win/All\ Plugins/Flurry/bin/Android/armeabi-v7a
cp plugins/Flurry/source/Android/libs/x86/libflurry.so $BUILD_DIR/win/All\ Plugins/Flurry/bin/Android/x86

mkdir $BUILD_DIR/win/All\ Plugins/Facebook/bin/Android
mkdir $BUILD_DIR/win/All\ Plugins/Facebook/bin/Android/armeabi
mkdir $BUILD_DIR/win/All\ Plugins/Facebook/bin/Android/armeabi-v7a
mkdir $BUILD_DIR/win/All\ Plugins/Facebook/bin/Android/x86
cp plugins/Facebook/source/Android/libs/armeabi/libfacebook.so $BUILD_DIR/win/All\ Plugins/Facebook/bin/Android/armeabi
cp plugins/Facebook/source/Android/libs/armeabi-v7a/libfacebook.so $BUILD_DIR/win/All\ Plugins/Facebook/bin/Android/armeabi-v7a
cp plugins/Facebook/source/Android/libs/x86/libfacebook.so $BUILD_DIR/win/All\ Plugins/Facebook/bin/Android/x86

mkdir $BUILD_DIR/win/All\ Plugins/Google\ Billing/bin/Android
mkdir $BUILD_DIR/win/All\ Plugins/Google\ Billing/bin/Android/armeabi
mkdir $BUILD_DIR/win/All\ Plugins/Google\ Billing/bin/Android/armeabi-v7a
mkdir $BUILD_DIR/win/All\ Plugins/Google\ Billing/bin/Android/x86
cp plugins/Google\ Billing/source/Android/libs/armeabi/libggooglebilling.so $BUILD_DIR/win/All\ Plugins/Google\ Billing/bin/Android/armeabi
cp plugins/Google\ Billing/source/Android/libs/armeabi-v7a/libggooglebilling.so $BUILD_DIR/win/All\ Plugins/Google\ Billing/bin/Android/armeabi-v7a
cp plugins/Google\ Billing/source/Android/libs/x86/libggooglebilling.so $BUILD_DIR/win/All\ Plugins/Google\ Billing/bin/Android/x86

mkdir $BUILD_DIR/win/Plugins
cp plugins/LuaSQLite3/source/release/lsqlite3.dll $BUILD_DIR/win/Plugins
cp plugins/LuaSocket/source/release/luasocket.dll $BUILD_DIR/win/Plugins
cp plugins/LuaFileSystem/source/release/lfs.dll $BUILD_DIR/win/Plugins
cp plugins/Microphone/source/Desktop/release/microphone.dll $BUILD_DIR/win/Plugins
cp plugins/BitOp/source/release/bitop.dll $BUILD_DIR/win/Plugins
cp plugins/JSON/source/release/json.dll $BUILD_DIR/win/Plugins





cd scripts


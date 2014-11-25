cd ..

rm -rf tmp
mkdir tmp
mkdir tmp/win

cp libgid/release/gid.dll tmp/win
cp libgvfs/release/gvfs.dll tmp/win
cp lua/release/lua.dll tmp/win
cp libgideros/release/gideros.dll tmp/win
cp libpystring/release/pystring.dll tmp/win

cp ui/release/GiderosStudio.exe tmp/win
cp player/release/GiderosPlayer.exe tmp/win
cp texturepacker/release/GiderosTexturePacker.exe tmp/win
cp fontcreator/release/GiderosFontCreator.exe tmp/win
cp licensemanager/release/GiderosLicenseManager.exe tmp/win

git archive -o tmp/tmp.tar HEAD:ui/Resources
mkdir tmp/win/Resources
tar xf tmp/tmp.tar -C tmp/win/Resources

git archive -o tmp/tmp.tar HEAD:ui/Tools
mkdir tmp/win/Tools
tar xf tmp/tmp.tar -C tmp/win/Tools

cp $QT_WIN/5.3/mingw482_32/bin/icudt52.dll tmp/win
cp $QT_WIN/5.3/mingw482_32/bin/icuin52.dll tmp/win
cp $QT_WIN/5.3/mingw482_32/bin/icuuc52.dll tmp/win
cp $QT_WIN/5.3/mingw482_32/bin/libgcc_s_dw2-1.dll tmp/win
cp $QT_WIN/5.3/mingw482_32/bin/libstdc++-6.dll tmp/win
cp $QT_WIN/5.3/mingw482_32/bin/libwinpthread-1.dll tmp/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Core.dll tmp/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Gui.dll tmp/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Network.dll tmp/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5OpenGL.dll tmp/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5PrintSupport.dll tmp/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Widgets.dll tmp/win
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Xml.dll tmp/win

mkdir tmp/win/imageformats
cp $QT_WIN/5.3/mingw482_32/plugins/imageformats/qjpeg.dll tmp/win/imageformats

mkdir tmp/win/platforms
cp $QT_WIN/5.3/mingw482_32/plugins/platforms/qminimal.dll tmp/win/platforms
cp $QT_WIN/5.3/mingw482_32/plugins/platforms/qoffscreen.dll tmp/win/platforms
cp $QT_WIN/5.3/mingw482_32/plugins/platforms/qwindows.dll tmp/win/platforms

cp $QT_WIN/5.3/mingw482_32/lib/qscintilla2.dll tmp/win

cp libgid/external/glew-1.10.0/lib/mingw48_32/glew32.dll tmp/win
cp libgid/external/openal-soft-1.13/build/mingw48_32/OpenAL32.dll tmp/win

cp $QT_WIN/5.3/mingw482_32/bin/icudt52.dll tmp/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/icuin52.dll tmp/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/icuuc52.dll tmp/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/libgcc_s_dw2-1.dll tmp/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/libstdc++-6.dll tmp/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/libwinpthread-1.dll tmp/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Core.dll tmp/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Network.dll tmp/win/Tools
cp $QT_WIN/5.3/mingw482_32/bin/Qt5Xml.dll tmp/win/Tools

mkdir tmp/win/Templates

git archive -o tmp/tmp.tar HEAD:ui/Templates/Eclipse
mkdir tmp/win/Templates/Eclipse
tar xf tmp/tmp.tar -C tmp/win/Templates/Eclipse

git archive -o tmp/tmp.tar HEAD:ui/Templates/Xcode4
mkdir tmp/win/Templates/Xcode4
tar xf tmp/tmp.tar -C tmp/win/Templates/Xcode4

mkdir tmp/win/Templates/Eclipse/Android\ Template/assets
mkdir tmp/win/Templates/Eclipse/Android\ Template/gen
mkdir tmp/win/Templates/Eclipse/Android\ Template/res/layout
mkdir tmp/win/Templates/Xcode4/iOS\ Template/iOS\ Template/assets

git archive -o tmp/tmp.tar HEAD:samplecode
mkdir tmp/win/Examples
tar xf tmp/tmp.tar -C tmp/win/Examples

git archive -o tmp/tmp.tar HEAD:ios/GiderosiOSPlayer
mkdir tmp/win/GiderosiOSPlayer
tar xf tmp/tmp.tar -C tmp/win/GiderosiOSPlayer

cp gdrdeamon/release/gdrdeamon.exe tmp/win/Tools
cp gdrbridge/release/gdrbridge.exe tmp/win/Tools
cp gdrexport/release/gdrexport.exe tmp/win/Tools

cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/giderosapi.h  tmp/win/Templates/Xcode4/iOS\ Template/iOS\ Template
cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/*.a           tmp/win/Templates/Xcode4/iOS\ Template/iOS\ Template
cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/giderosapi.h  tmp/win/GiderosiOSPlayer/GiderosiOSPlayer
cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/*.a           tmp/win/GiderosiOSPlayer/GiderosiOSPlayer

mkdir tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp Sdk/include/*.h tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/Game\ Kit/source/iOS/gamekit.mm tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/Store\ Kit/source/iOS/storekit.mm tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/iAd/source/iOS/iad.mm tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaSQLite3/source/lsqlite3.c tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaSQLite3/source/lsqlite3_stub.cpp  tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaSocket/source/luasocket_stub.cpp tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp ui/Templates/Xcode4/iOS\ Template/iOS\ Template/Plugins/libluasocket.a tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaFileSystem/source/lfs.h tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaFileSystem/source/lfs.c tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/LuaFileSystem/source/lfs_stub.cpp tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/BitOp/source/bit.c tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/BitOp/source/bit_stub.cpp tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/JSON/source/fpconv.c tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/JSON/source/fpconv.h tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/JSON/source/strbuf.c tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/JSON/source/strbuf.h tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/JSON/source/lua_cjson.c tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
cp plugins/JSON/source/lua_cjson_stub.cpp tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
mkdir tmp/win/Templates/Xcode4/iOS\ Template/iOS\ Template/Plugins
cp tmp/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins/* tmp/win/Templates/Xcode4/iOS\ Template/iOS\ Template/Plugins


cp android/GiderosAndroidPlayer/gideros.jar tmp/win/Templates/Eclipse/Android\ Template
mkdir tmp/win/Templates/Eclipse/Android\ Template/jni
cp android/lib/jni/Application.mk tmp/win/Templates/Eclipse/Android\ Template/jni
cp -R android/build/libs tmp/win/Templates/Eclipse/Android\ Template


cp plugins/LuaSocket/source/libs/armeabi/libluasocket.so tmp/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/LuaSocket/source/libs/armeabi-v7a/libluasocket.so tmp/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/LuaSocket/source/libs/x86/libluasocket.so tmp/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/LuaFileSystem/source/libs/armeabi/liblfs.so tmp/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/LuaFileSystem/source/libs/armeabi-v7a/liblfs.so tmp/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/LuaFileSystem/source/libs/x86/liblfs.so tmp/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/Google\ Billing/source/Android/libs/armeabi/libggooglebilling.so tmp/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/Google\ Billing/source/Android/libs/armeabi-v7a/libggooglebilling.so tmp/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/Google\ Billing/source/Android/libs/x86/libggooglebilling.so tmp/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/LuaSQLite3/source/libs/armeabi/liblsqlite3.so tmp/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/LuaSQLite3/source/libs/armeabi-v7a/liblsqlite3.so tmp/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/LuaSQLite3/source/libs/x86/liblsqlite3.so tmp/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/BitOp/source/libs/armeabi/libbitop.so tmp/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/BitOp/source/libs/armeabi-v7a/libbitop.so tmp/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/BitOp/source/libs/x86/libbitop.so tmp/win/Templates/Eclipse/Android\ Template/libs/x86

cp plugins/JSON/source/libs/armeabi/libjson.so tmp/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp plugins/JSON/source/libs/armeabi-v7a/libjson.so tmp/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp plugins/JSON/source/libs/x86/libjson.so tmp/win/Templates/Eclipse/Android\ Template/libs/x86

git archive -o tmp/tmp.tar HEAD:plugins/Google\ Billing/source/Android/com
tar xf tmp/tmp.tar -C tmp/win/Templates/Eclipse/Android\ Template/src/com
tar xf tmp/tmp.tar -C android/GiderosAndroidPlayer/src/com

rm -rf android/GiderosAndroidPlayer/libs
cp -R tmp/win/Templates/Eclipse/Android\ Template/libs android/GiderosAndroidPlayer

cd android/GiderosAndroidPlayer
ant debug
mv bin/GiderosAndroidPlayer-debug.apk ../../tmp/win/GiderosAndroidPlayer.apk
cd ../..

cd tmp/win
zip -r GiderosiOSPlayer.zip GiderosiOSPlayer
rm -rf GiderosiOSPlayer
cd ../..


git archive -o tmp/tmp.tar HEAD:doc
mkdir tmp/win/Documentation
tar xf tmp/tmp.tar -C tmp/win/Documentation

cp licenses.txt tmp/win

cp -R Sdk tmp/win


git archive -o tmp/tmp.tar HEAD:plugins
mkdir tmp/win/All\ Plugins
tar xf tmp/tmp.tar -C tmp/win/All\ Plugins

mkdir tmp/win/All\ Plugins/BitOp/bin
mkdir tmp/win/All\ Plugins/Facebook/bin
mkdir tmp/win/All\ Plugins/Flurry/bin
mkdir tmp/win/All\ Plugins/Game\ Kit/bin
mkdir tmp/win/All\ Plugins/Google\ Billing/bin
mkdir tmp/win/All\ Plugins/iAd/bin
mkdir tmp/win/All\ Plugins/JSON/bin
mkdir tmp/win/All\ Plugins/LPeg/bin
mkdir tmp/win/All\ Plugins/LuaFileSystem/bin
mkdir tmp/win/All\ Plugins/LuaSocket/bin
mkdir tmp/win/All\ Plugins/LuaSQLite3/bin
mkdir tmp/win/All\ Plugins/Microphone/bin
mkdir tmp/win/All\ Plugins/Store\ Kit/bin
mkdir tmp/win/All\ Plugins/BitOp/bin/Windows
cp plugins/BitOp/source/release/bitop.dll tmp/win/All\ Plugins/BitOp/bin/Windows
mkdir tmp/win/All\ Plugins/LuaSQLite3/bin/Windows
cp plugins/LuaSQLite3/source/release/lsqlite3.dll tmp/win/All\ Plugins/LuaSQLite3/bin/Windows
mkdir tmp/win/All\ Plugins/LuaSocket/bin/Windows
cp plugins/LuaSocket/source/release/luasocket.dll tmp/win/All\ Plugins/LuaSocket/bin/Windows
mkdir tmp/win/All\ Plugins/LuaFileSystem/bin/Windows
cp plugins/LuaFileSystem/source/release/lfs.dll tmp/win/All\ Plugins/LuaFileSystem/bin/Windows
mkdir tmp/win/All\ Plugins/LPeg/bin/Windows
cp plugins/LPeg/source/release/lpeg.dll tmp/win/All\ Plugins/LPeg/bin/Windows
mkdir tmp/win/All\ Plugins/Microphone/bin/Windows
cp plugins/Microphone/source/Desktop/release/microphone.dll tmp/win/All\ Plugins/Microphone/bin/Windows
mkdir tmp/win/All\ Plugins/JSON/bin/Windows
cp plugins/JSON/source/release/json.dll tmp/win/All\ Plugins/JSON/bin/Windows


mkdir tmp/win/All\ Plugins/BitOp/bin/Android
mkdir tmp/win/All\ Plugins/BitOp/bin/Android/armeabi
mkdir tmp/win/All\ Plugins/BitOp/bin/Android/armeabi-v7a
mkdir tmp/win/All\ Plugins/BitOp/bin/Android/x86
cp plugins/BitOp/source/libs/armeabi/libbitop.so tmp/win/All\ Plugins/BitOp/bin/Android/armeabi
cp plugins/BitOp/source/libs/armeabi-v7a/libbitop.so tmp/win/All\ Plugins/BitOp/bin/Android/armeabi-v7a
cp plugins/BitOp/source/libs/x86/libbitop.so tmp/win/All\ Plugins/BitOp/bin/Android/x86

mkdir tmp/win/All\ Plugins/LuaSocket/bin/Android
mkdir tmp/win/All\ Plugins/LuaSocket/bin/Android/armeabi
mkdir tmp/win/All\ Plugins/LuaSocket/bin/Android/armeabi-v7a
mkdir tmp/win/All\ Plugins/LuaSocket/bin/Android/x86
cp plugins/LuaSocket/source/libs/armeabi/libluasocket.so tmp/win/All\ Plugins/LuaSocket/bin/Android/armeabi
cp plugins/LuaSocket/source/libs/armeabi-v7a/libluasocket.so tmp/win/All\ Plugins/LuaSocket/bin/Android/armeabi-v7a
cp plugins/LuaSocket/source/libs/x86/libluasocket.so tmp/win/All\ Plugins/LuaSocket/bin/Android/x86

mkdir tmp/win/All\ Plugins/LPeg/bin/Android
mkdir tmp/win/All\ Plugins/LPeg/bin/Android/armeabi
mkdir tmp/win/All\ Plugins/LPeg/bin/Android/armeabi-v7a
mkdir tmp/win/All\ Plugins/LPeg/bin/Android/x86
cp plugins/LPeg/source/libs/armeabi/liblpeg.so tmp/win/All\ Plugins/LPeg/bin/Android/armeabi
cp plugins/LPeg/source/libs/armeabi-v7a/liblpeg.so tmp/win/All\ Plugins/LPeg/bin/Android/armeabi-v7a
cp plugins/LPeg/source/libs/x86/liblpeg.so tmp/win/All\ Plugins/LPeg/bin/Android/x86

mkdir tmp/win/All\ Plugins/LuaFileSystem/bin/Android
mkdir tmp/win/All\ Plugins/LuaFileSystem/bin/Android/armeabi
mkdir tmp/win/All\ Plugins/LuaFileSystem/bin/Android/armeabi-v7a
mkdir tmp/win/All\ Plugins/LuaFileSystem/bin/Android/x86
cp plugins/LuaFileSystem/source/libs/armeabi/liblfs.so tmp/win/All\ Plugins/LuaFileSystem/bin/Android/armeabi
cp plugins/LuaFileSystem/source/libs/armeabi-v7a/liblfs.so tmp/win/All\ Plugins/LuaFileSystem/bin/Android/armeabi-v7a
cp plugins/LuaFileSystem/source/libs/x86/liblfs.so tmp/win/All\ Plugins/LuaFileSystem/bin/Android/x86

mkdir tmp/win/All\ Plugins/LuaSQLite3/bin/Android
mkdir tmp/win/All\ Plugins/LuaSQLite3/bin/Android/armeabi
mkdir tmp/win/All\ Plugins/LuaSQLite3/bin/Android/armeabi-v7a
mkdir tmp/win/All\ Plugins/LuaSQLite3/bin/Android/x86
cp plugins/LuaSQLite3/source/libs/armeabi/liblsqlite3.so tmp/win/All\ Plugins/LuaSQLite3/bin/Android/armeabi
cp plugins/LuaSQLite3/source/libs/armeabi-v7a/liblsqlite3.so tmp/win/All\ Plugins/LuaSQLite3/bin/Android/armeabi-v7a
cp plugins/LuaSQLite3/source/libs/x86/liblsqlite3.so tmp/win/All\ Plugins/LuaSQLite3/bin/Android/x86

mkdir tmp/win/All\ Plugins/Microphone/bin/Android
mkdir tmp/win/All\ Plugins/Microphone/bin/Android/armeabi
mkdir tmp/win/All\ Plugins/Microphone/bin/Android/armeabi-v7a
mkdir tmp/win/All\ Plugins/Microphone/bin/Android/x86
cp plugins/Microphone/source/Android/libs/armeabi/libmicrophone.so tmp/win/All\ Plugins/Microphone/bin/Android/armeabi
cp plugins/Microphone/source/Android/libs/armeabi-v7a/libmicrophone.so tmp/win/All\ Plugins/Microphone/bin/Android/armeabi-v7a
cp plugins/Microphone/source/Android/libs/x86/libmicrophone.so tmp/win/All\ Plugins/Microphone/bin/Android/x86

mkdir tmp/win/All\ Plugins/JSON/bin/Android
mkdir tmp/win/All\ Plugins/JSON/bin/Android/armeabi
mkdir tmp/win/All\ Plugins/JSON/bin/Android/armeabi-v7a
mkdir tmp/win/All\ Plugins/JSON/bin/Android/x86
cp plugins/JSON/source/libs/armeabi/libjson.so tmp/win/All\ Plugins/JSON/bin/Android/armeabi
cp plugins/JSON/source/libs/armeabi-v7a/libjson.so tmp/win/All\ Plugins/JSON/bin/Android/armeabi-v7a
cp plugins/JSON/source/libs/x86/libjson.so tmp/win/All\ Plugins/JSON/bin/Android/x86

mkdir tmp/win/All\ Plugins/Flurry/bin/Android
mkdir tmp/win/All\ Plugins/Flurry/bin/Android/armeabi
mkdir tmp/win/All\ Plugins/Flurry/bin/Android/armeabi-v7a
mkdir tmp/win/All\ Plugins/Flurry/bin/Android/x86
cp plugins/Flurry/source/Android/libs/armeabi/libflurry.so tmp/win/All\ Plugins/Flurry/bin/Android/armeabi
cp plugins/Flurry/source/Android/libs/armeabi-v7a/libflurry.so tmp/win/All\ Plugins/Flurry/bin/Android/armeabi-v7a
cp plugins/Flurry/source/Android/libs/x86/libflurry.so tmp/win/All\ Plugins/Flurry/bin/Android/x86

mkdir tmp/win/All\ Plugins/Facebook/bin/Android
mkdir tmp/win/All\ Plugins/Facebook/bin/Android/armeabi
mkdir tmp/win/All\ Plugins/Facebook/bin/Android/armeabi-v7a
mkdir tmp/win/All\ Plugins/Facebook/bin/Android/x86
cp plugins/Facebook/source/Android/libs/armeabi/libfacebook.so tmp/win/All\ Plugins/Facebook/bin/Android/armeabi
cp plugins/Facebook/source/Android/libs/armeabi-v7a/libfacebook.so tmp/win/All\ Plugins/Facebook/bin/Android/armeabi-v7a
cp plugins/Facebook/source/Android/libs/x86/libfacebook.so tmp/win/All\ Plugins/Facebook/bin/Android/x86

mkdir tmp/win/All\ Plugins/Google\ Billing/bin/Android
mkdir tmp/win/All\ Plugins/Google\ Billing/bin/Android/armeabi
mkdir tmp/win/All\ Plugins/Google\ Billing/bin/Android/armeabi-v7a
mkdir tmp/win/All\ Plugins/Google\ Billing/bin/Android/x86
cp plugins/Google\ Billing/source/Android/libs/armeabi/libggooglebilling.so tmp/win/All\ Plugins/Google\ Billing/bin/Android/armeabi
cp plugins/Google\ Billing/source/Android/libs/armeabi-v7a/libggooglebilling.so tmp/win/All\ Plugins/Google\ Billing/bin/Android/armeabi-v7a
cp plugins/Google\ Billing/source/Android/libs/x86/libggooglebilling.so tmp/win/All\ Plugins/Google\ Billing/bin/Android/x86

mkdir tmp/win/Plugins
cp plugins/LuaSQLite3/source/release/lsqlite3.dll tmp/win/Plugins
cp plugins/LuaSocket/source/release/luasocket.dll tmp/win/Plugins
cp plugins/LuaFileSystem/source/release/lfs.dll tmp/win/Plugins
cp plugins/Microphone/source/Desktop/release/microphone.dll tmp/win/Plugins
cp plugins/BitOp/source/release/bitop.dll tmp/win/Plugins
cp plugins/JSON/source/release/json.dll tmp/win/Plugins





cd scripts


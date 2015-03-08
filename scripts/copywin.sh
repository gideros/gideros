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

cd plugins

for d in *; do
cd $d/source
if [ -d iOS ] ; then
cp -r iOS/* ../../../$BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
else
if [ $d != luasocket ] ; then
cp *.{h,c,cpp,m,mm,a} ../../../$BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins
fi
fi
cd ..
cd ..
done
cd ..

mkdir $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/iOS\ Template/Plugins
cp $BUILD_DIR/win/GiderosiOSPlayer/GiderosiOSPlayer/Plugins/* $BUILD_DIR/win/Templates/Xcode4/iOS\ Template/iOS\ Template/Plugins


cp android/GiderosAndroidPlayer/gideros.jar $BUILD_DIR/win/Templates/Eclipse/Android\ Template
mkdir $BUILD_DIR/win/Templates/Eclipse/Android\ Template/jni
cp android/lib/jni/Application.mk $BUILD_DIR/win/Templates/Eclipse/Android\ Template/jni
cp -R android/build/libs $BUILD_DIR/win/Templates/Eclipse/Android\ Template

cd plugins
for d in *; do
cd $d/source
if [ -d Android ] || [ -d jni ] ; then
if [ -d Android ]; then
cd Android
cp libs/armeabi/lib$d.so ../../../../$BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp libs/armeabi-v7a/lib$d.so ../../../../$BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp libs/x86/lib$d.so ../../../../$BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86
cd ..
else
cp libs/armeabi/lib$d.so ../../../$BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi
cp libs/armeabi-v7a/lib$d.so ../../../$BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/armeabi-v7a
cp libs/x86/lib$d.so ../../../$BUILD_DIR/win/Templates/Eclipse/Android\ Template/libs/x86
fi
fi
cd ..
cd ..
done
cd ..

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

mkdir $BUILD_DIR/win/Plugins

cd plugins
for d in *; do
cd $d/source
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin
if [ -f $d.pro ]; then
cp release/$d.dll ../../../$BUILD_DIR/win/Plugins
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Windows
cp release/$d.dll ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Windows
fi
if [ -d Android ] || [ -d jni ] ; then
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/armeabi
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/armeabi-v7a
mkdir ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/x86
if [ -d Android ] ; then
cd Android
cp libs/armeabi/lib$d.so ../../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/armeabi
cp libs/armeabi-v7a/lib$d.so ../../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/armeabi-v7a
cp libs/x86/lib$d.so ../../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/x86
cd ..
else
cp libs/armeabi/lib$d.so ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/armeabi
cp libs/armeabi-v7a/lib$d.so ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/armeabi-v7a
cp libs/x86/lib$d.so ../../../$BUILD_DIR/win/All\ Plugins/$d/bin/Android/x86
fi
fi
cd ..
cd ..
done
cd ..

cd scripts


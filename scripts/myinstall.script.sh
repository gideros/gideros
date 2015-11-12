rm -rf ~/.wine
wine xyz > /dev/null 2>&1

export QT=/Users/ar2rsawseen/Qt/5.5/clang_64
export QT_WIN=~/.wine/drive_c/Qt/Qt5.5.0/5.5/mingw492_32
export QT_DLL=54
export IOS_SDK=9.1
export ANDROID_HOME=/usr/local/opt/android-sdk
export ANDROID_NDK=/usr/local/opt/android-ndk
export GVERSION=2015.02

rm -rf build
mkdir build

cd scripts
echo 'Updating api annotation'
wget "http://docs.giderosmobile.com/reference/autocomplete.php" -O ../ui/Resources/gideros_annot.api

echo 'Installing Qt for Windows...'
tar zxf ../../dependencies/Qt.tar.bz2 -C ~/.wine/drive_c

echo 'Installing NSIS for Windows...'
tar zxf ../../dependencies/NSIS.tar.bz2 -C ~/.wine/drive_c

echo 'Installing QScintilla for Windows...'
tar zxf ../../dependencies/QScintilla-gpl-2.8.4.tar.gz -C ../build/
wine cmd /c installqscintilla.bat > /dev/null 2>&1

echo 'Installing QScintilla for Mac...'
rm -rf ../build/QScintilla-gpl-2.8.4
tar zxf ../../dependencies/QScintilla-gpl-2.8.4.tar.gz -C ../build/
bash installqscintilla.sh > /dev/null

echo 'Building Qt applications for Windows...'
rm -rf ../Sdk
rm -rf ../gdrdeamon/qtsinglecoreapplication.o
wine cmd /c qt5\\buildqtlibs.bat > /dev/null
wine cmd /c qt5\\buildplugins.bat > /dev/null
wine cmd /c qt5\\cleanqt.bat > /dev/null
wine cmd /c qt5\\buildqt.bat > /dev/null

echo 'Building Qt applications for Mac...'
rm -rf ../Sdk
bash qt5/buildqtlibs.sh > /dev/null
bash qt5/buildplugins.sh > /dev/null
bash qt5/cleanqt.sh > /dev/null
bash qt5/buildqt.sh > /dev/null

echo 'Building iOS libraries...'
bash cleanioslibs.sh > /dev/null
bash buildioslibs.sh > /dev/null
bash buildiosplugins.sh > /dev/null
bash cleanatvlibs.sh > /dev/null
bash buildatvlibs.sh > /dev/null
bash buildatvplugins.sh > /dev/null

echo 'Building Android libraries...'
bash makejar.sh > /dev/null
bash buildandroidlibs.sh > /dev/null
bash buildandroidso.sh > /dev/null
bash buildandroidplugins.sh > /dev/null

echo 'Copying Windows files...'
bash copywin.sh > /dev/null

echo 'Copying Mac files...'
bash copymac.sh

cp -R ../../external/VisualStudio ../build/win/Templates
cp -R ../../external/win32 ../build/win/Templates
cp -R ../../external/GiderosWindowsPhonePlayer.zip ../build/win
cp -R ../../external/GiderosWindowsPlayer.zip ../build/win
cp -R ../../external/VisualStudio ../build/mac/Gideros\ Studio.app/Contents/Templates
cp -R ../../external/win32 ../build/mac/Gideros\ Studio.app/Contents/Templates
cp -R ../../external/GiderosWindowsPhonePlayer.zip ../build/mac
cp -R ../../external/GiderosWindowsPlayer.zip ../build/mac

echo 'Creating Windows installation package...'
bash createwinpackage.sh > /dev/null

echo 'Creating Mac installation package...'
bash createmacpackage.sh





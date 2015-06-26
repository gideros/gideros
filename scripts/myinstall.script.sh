rm -rf ~/.wine
wine xyz 

export QT=/usr/local/Cellar/qt5/5.4.0
export QT_WIN=~/.wine/drive_c/Qt/Qt5.4.2
export IOS_SDK=8.2
export ANDROID_HOME=/usr/local/opt/android-sdk
export ANDROID_NDK=/usr/local/opt/android-ndk
export GVERSION=2015.02

rm -rf build
mkdir build

cd scripts

echo 'Installing Qt for Windows...'
tar zxf ../../dependencies/Qt5.4.2.tar.bz2 -C ~/.wine/drive_c

echo 'Installing NSIS for Windows...'
tar zxf ../../dependencies/NSIS.tar.bz2 -C ~/.wine/drive_c

echo 'Installing QScintilla for Windows...'
tar zxvf ../../dependencies/QScintilla-gpl-2.8.4.tar.gz -C ../build/
wine cmd /c installqscintilla.bat

echo 'Building Qt applications for Windows...'
rm -rf ../Sdk
wine cmd /c qt5\\buildqtlibs.bat > /dev/null
wine cmd /c qt5\\buildplugins.bat > /dev/null
wine cmd /c qt5\\cleanqt.bat > /dev/null
wine cmd /c qt5\\buildqt.bat > /dev/null

echo 'Building iOS libraries...'
bash cleanioslibs.sh > /dev/null
bash buildioslibs.sh > /dev/null
bash buildiosplugins.sh > /dev/null

echo 'Building Android libraries...'
bash makejar.sh > /dev/null
bash buildandroidlibs.sh > /dev/null
bash buildandroidso.sh > /dev/null
bash buildandroidplugins.sh > /dev/null

echo 'Copying Windows files...'
bash copywin.sh > /dev/null

echo 'Creating Windows installation package...'
bash createwinpackage.sh > /dev/null


echo 'Installing QScintilla for Mac...'
rm -rf ../build/QScintilla-gpl-2.8.4
tar zxvf ../../dependencies/QScintilla-gpl-2.8.4.tar.gz -C ../build/
bash installqscintilla.sh > /dev/null

echo 'Building Qt applications for Mac...'
rm -rf ../Sdk
bash qt5/buildqtlibs.sh > /dev/null
bash qt5/buildplugins.sh > /dev/null
bash qt5/cleanqt.sh > /dev/null
bash qt5/buildqt.sh > /dev/null


echo 'Copying Mac files...'
bash copymac.sh

echo 'Creating Mac installation package...'
bash createmacpackage.sh





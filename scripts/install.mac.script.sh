brew update
brew install freetype
brew install glew
brew install qt5
brew install ant
brew install android-sdk
brew install android-ndk

rm -rf ~/.wine
wine xyz 

expect -c '
set timeout -1   ;
spawn android update sdk -u -t tools,platform-tools,build-tools-21.1.1,android-21; 
expect { 
    "Do you accept the license" { exp_send "y\r" ; exp_continue }
    eof
}
'

export QT=/usr/local/Cellar/qt5/5.3.2
export QT_WIN=~/.wine/drive_c/Qt/Qt5.3.2
export IOS_SDK=8.1
export ANDROID_HOME=/usr/local/opt/android-sdk
export ANDROID_NDK=/usr/local/opt/android-ndk

rm -rf build
mkdir build

cd scripts

echo 'Building iOS libraries...'
bash cleanioslibs.sh
bash buildioslibs.sh
bash buildiosplugins.sh

echo 'Building Android libraries...'
bash makejar.sh
bash buildandroidlibs.sh
bash buildandroidso.sh
bash buildandroidplugins.sh


echo 'Installing QScintilla for Mac...'
bash downloadqscintilla.sh
bash extractqscintilla.sh
bash installqscintilla.sh

echo 'Building Qt applications for Mac...'
rm -rf ../Sdk
bash qt5/buildqtlibs.sh
bash qt5/buildplugins.sh
bash qt5/cleanqt.sh
bash qt5/buildqt.sh


echo 'Copying Mac files...'
bash copymac.sh

echo 'Creating Mac installation package...'
bash createmacpackage.sh





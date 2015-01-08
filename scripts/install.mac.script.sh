DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

brew update &> /dev/null
brew install freetype &> /dev/null
brew install glew &> /dev/null
brew install qt5 &> /dev/null
brew install ant &> /dev/null
brew install android-sdk &> /dev/null
brew install android-ndk &> /dev/null

expect -c '
set timeout -1   ;
spawn android update sdk -u -t tools,platform-tools,build-tools-21.1.1,android-21; 
expect { 
    "Do you accept the license" { exp_send "y\r" ; exp_continue }
    eof
}
'

export QT=/usr/local/Cellar/qt5/5.3.2
export IOS_SDK=8.1
export ANDROID_HOME=/usr/local/opt/android-sdk
export ANDROID_NDK=/usr/local/opt/android-ndk

rm -rf $DIR/build
mkdir $DIR/build

echo 'Building iOS libraries...'
bash $DIR/cleanioslibs.sh
bash $DIR/buildioslibs.sh
bash $DIR/buildiosplugins.sh

echo 'Building Android libraries...'
bash $DIR/makejar.sh
bash $DIR/buildandroidlibs.sh
bash $DIR/buildandroidso.sh
bash $DIR/buildandroidplugins.sh


echo 'Installing QScintilla for Mac...'
bash $DIR/downloadqscintilla.sh
bash $DIR/extractqscintilla.sh
bash $DIR/installqscintilla.sh

echo 'Building Qt applications for Mac...'
rm -rf $DIR/../Sdk
bash $DIR/qt5/buildqtlibs.sh
bash $DIR/qt5/buildplugins.sh
bash $DIR/qt5/cleanqt.sh
bash $DIR/qt5/buildqt.sh


echo 'Copying Mac files...'
bash $DIR/copymac.sh

echo 'Creating Mac installation package...'
bash $DIR/createmacpackage.sh





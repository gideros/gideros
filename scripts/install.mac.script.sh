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

rm -rf $DIR/../build
mkdir $DIR/../build

(
cd $DIR/../
echo 'Building iOS libraries...'
bash cleanioslibs.sh
bash buildioslibs.sh
bash buildiosplugins.sh
) &
(
cd $DIR/../
echo 'Building Android libraries...'
bash makejar.sh
bash buildandroidlibs.sh
bash buildandroidso.sh
bash buildandroidplugins.sh
) &
(
cd $DIR/../
echo 'Installing QScintilla for Mac...'
bash downloadqscintilla.sh
bash extractqscintilla.sh
bash installqscintilla.sh
) &
(
cd $DIR/../
echo 'Building Qt applications for Mac...'
rm -rf Sdk
bash qt5/buildqtlibs.sh
bash qt5/buildplugins.sh
bash qt5/cleanqt.sh
bash qt5/buildqt.sh
) &
wait

(
cd $DIR/../
echo 'Copying Mac files...'
bash copymac.sh

echo 'Creating Mac installation package...'
bash createmacpackage.sh
)





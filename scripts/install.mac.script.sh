DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo 'Updating brew';
brew update &> /dev/null
echo 'Finished updating brew';
echo 'Installing dependencies';
(brew install freetype) &
(brew install glew) &
(brew install qt5) &
(brew install ant) &
(brew install android-ndk) &
wait

echo 'Finished installing dependencies';

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
cd $DIR

echo 'Building libs';
(
echo 'Building iOS libraries...'
bash cleanioslibs.sh &> /dev/null
bash buildioslibs.sh &> /dev/null
bash buildiosplugins.sh &> /dev/null
) &
(
echo 'Building Android libraries...'
bash makejar.sh &> /dev/null
bash buildandroidlibs.sh &> /dev/null
bash buildandroidso.sh &> /dev/null
bash buildandroidplugins.sh &> /dev/null
) &
(
echo 'Installing QScintilla for Mac...'
bash downloadqscintilla.sh &> /dev/null
bash extractqscintilla.sh &> /dev/null
bash installqscintilla.sh &> /dev/null
) &
(
echo 'Building Qt applications for Mac...'
rm -rf Sdk
bash qt5/buildqtlibs.sh &> /dev/null
bash qt5/buildplugins.sh &> /dev/null
bash qt5/cleanqt.sh &> /dev/null
bash qt5/buildqt.sh &> /dev/null
) &
wait

echo 'Finished building libs';

echo 'Copying Mac files...'
bash copymac.sh

echo 'Creating Mac installation package...'
bash createmacpackage.sh





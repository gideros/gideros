echo 'Updating brew'
brew update &> /dev/null
echo 'Finished updating brew'
echo 'Installing dependencies'
(brew install freetype &> /dev/null) &
(brew install glew &> /dev/null) &
(brew install wine &> /dev/null) &
(brew install ant &> /dev/null) &
(brew install android-sdk &> /dev/null) &
(brew install android-ndk &> /dev/null) &
wait
echo 'Fiished installing dependencies'
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
(
echo 'Installing Qt for Windows...'
bash installwinqt.sh
) &
(
echo 'Installing NSIS for Windows...'
bash installnsis.sh
) &
(
echo 'Installing QScintilla for Windows...'
bash downloadqscintilla.sh
bash extractqscintilla.sh
wine cmd /c installqscintilla.bat
) &
(
echo 'Building Qt applications for Windows...'
rm -rf ../Sdk
wine cmd /c qt5\\buildqtlibs.bat
wine cmd /c qt5\\buildplugins.bat
wine cmd /c qt5\\cleanqt.bat
wine cmd /c qt5\\buildqt.bat
) &
(
echo 'Building iOS libraries...'
bash cleanioslibs.sh
bash buildioslibs.sh
bash buildiosplugins.sh
) &
(
echo 'Building Android libraries...'
bash makejar.sh
bash buildandroidlibs.sh
bash buildandroidso.sh
bash buildandroidplugins.sh
) &
wait

echo 'Copying Windows files...'
bash copywin.sh

echo 'Creating Windows installation package...'
bash createwinpackage.sh

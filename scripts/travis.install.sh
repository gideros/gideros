echo 'Updating brew'
brew update &> /dev/null
echo 'Finished updating brew'
echo 'Installing dependencies'
(brew install jpeg &> /dev/null) &
(brew install libpng &> /dev/null) &
(brew install freetype &> /dev/null) &
(brew install glew &> /dev/null) &
(brew install wget &> /dev/null) &
(brew install qt5 &> /dev/null) &
(brew install wine &> /dev/null) &
(brew install ant &> /dev/null) &
(brew install android-sdk &> /dev/null
brew install android-ndk &> /dev/null) &
wait
echo 'Finished installing dependencies'
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

export QT=/usr/local/Cellar/qt5/5.4.0
export QT_WIN=~/.wine/drive_c/Qt/Qt5.3.2
export IOS_SDK=8.2
export ANDROID_HOME=/usr/local/opt/android-sdk
export ANDROID_NDK=/usr/local/opt/android-ndk

rm -rf build
mkdir build

cd scripts
(
echo 'Installing NSIS for Windows...'
bash installnsis.sh &> /dev/null
) &
(
echo 'Installing Qt for Windows...'
bash installwinqt.sh &> /dev/null

echo 'Installing QScintilla for Windows...'
bash downloadqscintilla.sh &> /dev/null
bash extractqscintilla.sh &> /dev/null
wine cmd /c installqscintilla.bat

echo 'Building Qt applications for Windows...'
rm -rf ../Sdk
wine cmd /c qt5\\buildqtlibs.bat
wine cmd /c qt5\\buildplugins.bat
wine cmd /c qt5\\cleanqt.bat
wine cmd /c qt5\\buildqt.bat
) &
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

echo 'Building Qt applications for Mac...'
rm -rf ../Sdk
bash qt5/buildqtlibs.sh &> /dev/null
bash qt5/buildplugins.sh &> /dev/null
bash qt5/cleanqt.sh &> /dev/null
bash qt5/buildqt.sh &> /dev/null
) &
wait

echo 'Copying Windows files...'
bash copywin.sh

echo 'Creating Windows installation package...'
bash createwinpackage.sh

echo 'Copying Mac files...'
bash copymac.sh

echo 'Creating Mac installation package...'
bash createmacpackage.sh





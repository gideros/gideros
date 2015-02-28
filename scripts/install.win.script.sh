echo 'Updating brew'
brew update &> /dev/null
echo 'Finished updating brew'
echo 'Installing dependencies'
brew install jpeg --universal
brew link --overwrite jpeg
brew install libpng --universal
brew link --overwrite libpng
brew install freetype --universal
brew install glew
brew install Caskroom/cask/xquartz
brew install wget
brew install qt5
brew install wine
brew install ant
brew install android-sdk
brew install android-ndk
wait
echo 'Finished installing dependencies'
rm -rf ~/.wine
wine xyz 

expect -c '
set timeout -1   ;
spawn android update sdk -u -a -t tool,platform-tool,3,android-21;
expect { 
    "Do you accept the license" { exp_send "y\r" ; exp_continue }
    eof
}
'

export QT=/usr/local/Cellar/qt5/5.4.0
export QT_WIN=~/.wine/drive_c/Qt/Qt5.3.2
export IOS_SDK=8.1
export ANDROID_HOME=/usr/local/opt/android-sdk
export ANDROID_NDK=/usr/local/opt/android-ndk

rm -rf build
mkdir build

cd scripts
echo 'Installing Qt for Windows...'
bash installwinqt.sh

echo 'Installing NSIS for Windows...'
bash installnsis.sh

echo 'Installing QScintilla for Windows...'
bash downloadqscintilla.sh
bash extractqscintilla.sh
wine cmd /c installqscintilla.bat

echo 'Building Qt applications for Windows...'
rm -rf ../Sdk
wine cmd /c qt5\\buildqtlibs.bat
wine cmd /c qt5\\buildplugins.bat
wine cmd /c qt5\\cleanqt.bat
wine cmd /c qt5\\buildqt.bat

#echo 'Building iOS libraries...'
#bash cleanioslibs.sh
#bash buildioslibs.sh
#bash buildiosplugins.sh

echo 'Building Android libraries...'
bash makejar.sh
bash buildandroidlibs.sh
bash buildandroidso.sh
bash buildandroidplugins.sh

echo 'Copying Windows files...'
bash copywin.sh

echo 'Creating Windows installation package...'
bash createwinpackage.sh

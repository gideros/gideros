echo "Android home:" $ANDROID_HOME 

brew update
brew install jpeg
brew install libpng
brew install freetype
brew install glew
brew install wget
brew install qt5
brew install wine
brew install ant

export QT=/usr/local/Cellar/qt5/5.3.2
export QT_WIN=~/.wine/drive_c/Qt/Qt5.3.2
export IOS_SDK=8.0
export ANDROID_NDK=~/android-ndk-r10c
export ANDROID_HOME=~/android-sdk-macosx

cd scripts

mkdir ../tmp

bash downloadqscintilla.sh
bash extractqscintilla.sh
wine cmd /c installqscintilla.bat

rm -rf ../Sdk
wine cmd /c qt5\\buildqtlibs.bat
wine cmd /c qt5\\buildplugins.bat
wine cmd /c qt5\\cleanqt.bat
wine cmd /c qt5\\buildqt.bat

bash cleanioslibs.sh
bash buildioslibs.sh
bash buildiosplugins.sh

bash makejar.sh
bash buildandroidlibs.sh
bash buildandroidso.sh
bash buildandroidplugins.sh


bash extractqscintilla.sh
bash installqscintilla.sh

rm -rf ../Sdk
bash qt5/buildqtlibs.sh
bash qt5/buildplugins.sh
bash qt5/cleanqt.sh
bash qt5/buildqt.sh

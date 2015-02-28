echo 'Updating brew'
brew update &> /dev/null
echo 'Finished updating brew'
echo 'Installing dependencies'
brew install freetype --universal
brew install glew
brew install wine

export QT=/usr/local/Cellar/qt5/5.4.0
export QT_WIN=~/.wine/drive_c/Qt/Qt5.3.2

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

echo 'Copying Windows files...'
bash copywin.sh

echo 'Creating Windows installation package...'
bash createwinpackage.sh

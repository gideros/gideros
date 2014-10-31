brew update
brew install jpeg
brew install libpng
brew install freetype
brew install glew
brew install wget
brew install qt5
cd scripts

export QT=/usr/local/Cellar/qt5/5.3.2

bash installqscintilla.sh

bash qt5/buildqtlibs.sh
bash qt5/buildplugins.sh
bash qt5/cleanqt.sh
bash qt5/buildqt.sh


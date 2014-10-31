brew update
brew install jpeg
brew install libpng
brew install freetype
brew tap homebrew/dupes
brew install zlib
brew install glew
brew install qt5
brew linkapps
cd scripts
bash ./qt5/buildqtlibs.sh
bash ./qt5/buildplugins.sh

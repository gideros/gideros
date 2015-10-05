#install_name_tool -change libgvfs.1.dylib @executable_path/../Frameworks/libgvfs.dylib "MacOSXDesktopTemplate.app/Contents/MacOS/MacOSXDesktopTemplate"
#install_name_tool -change libpystring.1.dylib @executable_path/../Frameworks/libpystring.dylib "MacOSXDesktopTemplate.app/Contents/MacOS/MacOSXDesktopTemplate"
sudo cp ../libgid/libgid.1.dylib /usr/lib
sudo cp ../libgvfs/libgvfs.1.dylib /usr/lib
sudo cp ../lua/liblua.1.dylib /usr/lib
sudo cp ../libgideros/libgideros.1.dylib /usr/lib
sudo cp ../libpystring/libpystring.1.dylib /usr/lib
sudo /usr/local/Cellar/qt5/5.4.0/bin/macdeployqt "MacOSXDesktopTemplate.app"
sudo rm /usr/lib/libgid.1.dylib
sudo rm /usr/lib/libgvfs.1.dylib
sudo rm /usr/lib/liblua.1.dylib
sudo rm /usr/lib/libgideros.1.dylib
sudo rm /usr/lib/libpystring.1.dylib
make TARGET_CFLAGS="-I../../libgvfs" TARGET_LIBS="-L../../libgvfs -lgvfs"
install_name_tool -change libgvfs.1.dylib @executable_path/../Frameworks/libgvfs.1.dylib src/libluajit.so
#rename libluajit.so to liblua.1.dylib and replace in the GiderosPlayer/Frameworks folder
install_name_tool -id @executable_path/../Frameworks/liblua.dylib liblua.dylib
install_name_tool -change libgvfs.1.dylib @executable_path/../Frameworks/libgvfs.dylib liblua.dylib
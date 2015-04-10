install_name_tool -id @executable_path/../Frameworks/libgideros.dylib libgideros.dylib
install_name_tool -change libgid.1.dylib @executable_path/../Frameworks/libgid.dylib libgideros.dylib
install_name_tool -change liblua.1.dylib @executable_path/../Frameworks/liblua.dylib libgideros.dylib
install_name_tool -change libpystring.1.dylib @executable_path/../Frameworks/libpystring.dylib libgideros.dylib
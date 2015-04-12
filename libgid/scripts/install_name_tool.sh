install_name_tool -id @executable_path/../Frameworks/libgid.dylib libgid.dylib
install_name_tool -change libgvfs.1.dylib @executable_path/../Frameworks/libgvfs.dylib libgid.dylib
install_name_tool -change /Applications/Qt/5.4/clang_64/lib/QtOpenGL.framework/Versions/5/QtOpenGL @executable_path/../Frameworks/QtOpenGL.framework/Versions/5/QtOpenGL libgid.dylib
install_name_tool -change /Applications/Qt/5.4/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets @executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets libgid.dylib
install_name_tool -change /Applications/Qt/5.4/clang_64/lib/QtGui.framework/Versions/5/QtGui @executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui libgid.dylib
install_name_tool -change /Applications/Qt/5.4/clang_64/lib/QtCore.framework/Versions/5/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore libgid.dylib
install_name_tool -change /Applications/Qt/5.4/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork @executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork libgid.dylib
cd ~/myprojects/gideros/ui-build-desktop/Gideros\ Studio.app/Contents/MacOS/
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtWebKit.framework/Versions/Current/QtWebKit .
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtXml.framework/Versions/Current/QtXml .
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtGui.framework/Versions/Current/QtGui .
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtCore.framework/Versions/Current/QtCore .
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtNetwork.framework/Versions/Current/QtNetwork .
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/libqscintilla2.6.1.0.dylib libqscintilla2.6.dylib
cp -R /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtGui.framework/Versions/Current/Resources/ ../Resources
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtWebKit.framework/Versions/Current/QtWebKit" @executable_path/QtWebKit Gideros\ Studio
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtXml.framework/Versions/Current/QtXml" @executable_path/QtXml Gideros\ Studio
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtGui.framework/Versions/Current/QtGui" @executable_path/QtGui Gideros\ Studio
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtCore.framework/Versions/Current/QtCore" @executable_path/QtCore Gideros\ Studio
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtNetwork.framework/Versions/Current/QtNetwork" @executable_path/QtNetwork Gideros\ Studio
install_name_tool -change libqscintilla2.6.dylib @executable_path/libqscintilla2.6.dylib Gideros\ Studio

cd ~/myprojects/gideros/player-build-desktop/Gideros\ Player.app/Contents/MacOS/
cp /opt/local/lib/libpng14.14.dylib .
cp /opt/local/lib/libmpg123.0.dylib .
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtOpenGL.framework/Versions/Current/QtOpenGL .
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtGui.framework/Versions/Current/QtGui .
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtCore.framework/Versions/Current/QtCore .
cp -R /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtGui.framework/Versions/Current/Resources/ ../Resources
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtGui.framework/Versions/Current/QtGui" @executable_path/QtGui Gideros\ Player
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtCore.framework/Versions/Current/QtCore" @executable_path/QtCore Gideros\ Player
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtOpenGL.framework/Versions/Current/QtOpenGL" @executable_path/QtOpenGL Gideros\ Player
install_name_tool -change "/opt/local/lib/libpng14.14.dylib" @executable_path/libpng14.14.dylib Gideros\ Player
install_name_tool -change "/opt/local/lib/libmpg123.0.dylib" @executable_path/libmpg123.0.dylib Gideros\ Player

cd ~/myprojects/gideros/texturepacker-build-desktop/Texture\ Packer.app/Contents/MacOS/
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtXml.framework/Versions/Current/QtXml .
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtGui.framework/Versions/Current/QtGui .
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtCore.framework/Versions/Current/QtCore .
cp -R /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtGui.framework/Versions/Current/Resources/ ../Resources
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtGui.framework/Versions/Current/QtGui" @executable_path/QtGui Texture\ Packer
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtCore.framework/Versions/Current/QtCore" @executable_path/QtCore Texture\ Packer
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtXml.framework/Versions/Current/QtXml" @executable_path/QtXml Texture\ Packer

cd ~/myprojects/gideros/fontcreator-build-desktop/Font\ Creator.app/Contents/MacOS/
cp /opt/local/lib/libfreetype.6.dylib .
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtGui.framework/Versions/Current/QtGui .
cp /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtCore.framework/Versions/Current/QtCore .
cp -R /Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtGui.framework/Versions/Current/Resources/ ../Resources
install_name_tool -change "/opt/local/lib/libfreetype.6.dylib" @executable_path/libfreetype.6.dylib Font\ Creator
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtGui.framework/Versions/Current/QtGui" @executable_path/QtGui Font\ Creator
install_name_tool -change "/Developer/QtSDK/1.1.1/Desktop/Qt/473/gcc/lib/QtCore.framework/Versions/Current/QtCore" @executable_path/QtCore Font\ Creator


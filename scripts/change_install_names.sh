# Gideros Studio - libqcocoa
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtPrintSupport.framework/Versions/5/QtPrintSupport" "@executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport" ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/platforms/libqcocoa.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/platforms/libqcocoa.dylib

# Gideros Studio - libqtaccessiblewidgets
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/accessible/libqtaccessiblewidgets.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/accessible/libqtaccessiblewidgets.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/accessible/libqtaccessiblewidgets.dylib

# #Gideros Studio - imageformats
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqgif.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqgif.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqico.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqico.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqjpeg.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqjpeg.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqmng.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqmng.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqtga.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqtga.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqtiff.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqtiff.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqwbmp.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqwbmp.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqgif.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/imageformats/libqgif.dylib

# Gideros Studio - libcocoaprintersupport
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtPrintSupport.framework/Versions/5/QtPrintSupport" "@executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport" ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/printsupport/libcocoaprintersupport.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/printsupport/libcocoaprintersupport.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/printsupport/libcocoaprintersupport.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns/printsupport/libcocoaprintersupport.dylib

# Gideros Studio - QtGui
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtGui.framework/Versions/5/QtGui

# Gideros Studio - QtNetwork
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtNetwork.framework/Versions/5/QtNetwork

# Gideros Studio - QtPrintSupport
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"                     ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport

# Gideros Studio - QtWidgets
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtWidgets.framework/Versions/5/QtWidgets

# Gideros Studio - QtXml
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtXml.framework/Versions/5/QtXml

# Gideros Player - QtOpenGL
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"                     ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks/QtOpenGL.framework/Versions/5/QtOpenGL
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks/QtOpenGL.framework/Versions/5/QtOpenGL
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks/QtOpenGL.framework/Versions/5/QtOpenGL

# Gideros Studio
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"                     ~/Desktop/release/Gideros\ Studio.app/Contents/MacOS/Gideros\ Studio
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/MacOS/Gideros\ Studio
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/MacOS/Gideros\ Studio
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork" "@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork"                     ~/Desktop/release/Gideros\ Studio.app/Contents/MacOS/Gideros\ Studio
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtXml.framework/Versions/5/QtXml" "@executable_path/../Frameworks/QtXml.framework/Versions/5/QtXml"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/MacOS/Gideros\ Studio
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtPrintSupport.framework/Versions/5/QtPrintSupport" "@executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport" ~/Desktop/release/Gideros\ Studio.app/Contents/MacOS/Gideros\ Studio

# Gideros Studio - libqscintilla2
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtPrintSupport.framework/Versions/5/QtPrintSupport" "@executable_path/../Frameworks/QtPrintSupport.framework/Versions/5/QtPrintSupport" ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/libqscintilla2.9.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"                     ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/libqscintilla2.9.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/libqscintilla2.9.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/libqscintilla2.9.dylib

# Gideros Player
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"                     ~/Desktop/release/Gideros\ Player.app/Contents/MacOS/Gideros\ Player
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Player.app/Contents/MacOS/Gideros\ Player
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Player.app/Contents/MacOS/Gideros\ Player
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork" "@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork"                     ~/Desktop/release/Gideros\ Player.app/Contents/MacOS/Gideros\ Player
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtOpenGL.framework/Versions/5/QtOpenGL" "@executable_path/../Frameworks/QtOpenGL.framework/Versions/5/QtOpenGL"                         ~/Desktop/release/Gideros\ Player.app/Contents/MacOS/Gideros\ Player

# Gideros Player - libgid
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"                     ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks/libgid.1.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks/libgid.1.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks/libgid.1.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork" "@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork"                     ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks/libgid.1.dylib
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtOpenGL.framework/Versions/5/QtOpenGL" "@executable_path/../Frameworks/QtOpenGL.framework/Versions/5/QtOpenGL"                         ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks/libgid.1.dylib

#Gideros Player - libpystring
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks/libpystring.1.dylib

cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns                             ~/Desktop/release/Gideros\ Player.app/Contents
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtGui.framework          ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtNetwork.framework      ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtPrintSupport.framework ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtWidgets.framework      ~/Desktop/release/Gideros\ Player.app/Contents/Frameworks

# Gideros Font Creator
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"                     ~/Desktop/release/Gideros\ Font\ Creator.app/Contents/MacOS/Gideros\ Font\ Creator
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Font\ Creator.app/Contents/MacOS/Gideros\ Font\ Creator
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Font\ Creator.app/Contents/MacOS/Gideros\ Font\ Creator

cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns                             ~/Desktop/release/Gideros\ Font\ Creator.app/Contents
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtGui.framework          ~/Desktop/release/Gideros\ Font\ Creator.app/Contents/Frameworks
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtWidgets.framework      ~/Desktop/release/Gideros\ Font\ Creator.app/Contents/Frameworks
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtPrintSupport.framework ~/Desktop/release/Gideros\ Font\ Creator.app/Contents/Frameworks

# Gideros Texture Packer
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"                     ~/Desktop/release/Gideros\ Texture\ Packer.app/Contents/MacOS/Gideros\ Texture\ Packer
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ Texture\ Packer.app/Contents/MacOS/Gideros\ Texture\ Packer
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Texture\ Packer.app/Contents/MacOS/Gideros\ Texture\ Packer
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtXml.framework/Versions/5/QtXml" "@executable_path/../Frameworks/QtXml.framework/Versions/5/QtXml"                                     ~/Desktop/release/Gideros\ Texture\ Packer.app/Contents/MacOS/Gideros\ Texture\ Packer

cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns                             ~/Desktop/release/Gideros\ Texture\ Packer.app/Contents
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtGui.framework          ~/Desktop/release/Gideros\ Texture\ Packer.app/Contents/Frameworks
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtWidgets.framework      ~/Desktop/release/Gideros\ Texture\ Packer.app/Contents/Frameworks
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtXml.framework          ~/Desktop/release/Gideros\ Texture\ Packer.app/Contents/Frameworks
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtPrintSupport.framework ~/Desktop/release/Gideros\ Texture\ Packer.app/Contents/Frameworks

# Gideros License Manager
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtWidgets.framework/Versions/5/QtWidgets" "@executable_path/../Frameworks/QtWidgets.framework/Versions/5/QtWidgets"                     ~/Desktop/release/Gideros\ License\ Manager.app/Contents/MacOS/Gideros\ License\ Manager
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtGui.framework/Versions/5/QtGui" "@executable_path/../Frameworks/QtGui.framework/Versions/5/QtGui"                                     ~/Desktop/release/Gideros\ License\ Manager.app/Contents/MacOS/Gideros\ License\ Manager
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ License\ Manager.app/Contents/MacOS/Gideros\ License\ Manager
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtXml.framework/Versions/5/QtXml" "@executable_path/../Frameworks/QtXml.framework/Versions/5/QtXml"                                     ~/Desktop/release/Gideros\ License\ Manager.app/Contents/MacOS/Gideros\ License\ Manager
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork" "@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork"                     ~/Desktop/release/Gideros\ License\ Manager.app/Contents/MacOS/Gideros\ License\ Manager

cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/PlugIns                             ~/Desktop/release/Gideros\ License\ Manager.app/Contents
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtGui.framework          ~/Desktop/release/Gideros\ License\ Manager.app/Contents/Frameworks
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtWidgets.framework      ~/Desktop/release/Gideros\ License\ Manager.app/Contents/Frameworks
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtXml.framework          ~/Desktop/release/Gideros\ License\ Manager.app/Contents/Frameworks
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtNetwork.framework      ~/Desktop/release/Gideros\ License\ Manager.app/Contents/Frameworks
cp -R ~/Desktop/release/Gideros\ Studio.app/Contents/Frameworks/QtPrintSupport.framework ~/Desktop/release/Gideros\ License\ Manager.app/Contents/Frameworks

#####

# gdrbridge
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork" "@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork"                     ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrbridge
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrbridge

#gdrdeamon
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtXml.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtXml.framework/Versions/5/QtXml"                                  ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrdeamon
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrdeamon
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork" "@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork"                     ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrdeamon

#gdrexport
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtXml.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                  ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrexport
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtCore.framework/Versions/5/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore"                                 ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrexport
install_name_tool -change "/Qt/5.1.0//5.1.0/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork" "@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork"                     ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrexport

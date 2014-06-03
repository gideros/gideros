/Qt/Qt5.3.0/5.3/clang_64/bin/macdeployqt ~/Desktop/release/Gideros\ Studio.app
/Qt/Qt5.3.0/5.3/clang_64/bin/macdeployqt ~/Desktop/release/Gideros\ Player.app
/Qt/Qt5.3.0/5.3/clang_64/bin/macdeployqt ~/Desktop/release/Gideros\ Texture\ Packer.app
/Qt/Qt5.3.0/5.3/clang_64/bin/macdeployqt ~/Desktop/release/Gideros\ Font\ Creator.app
/Qt/Qt5.3.0/5.3/clang_64/bin/macdeployqt ~/Desktop/release/Gideros\ License\ Manager.app

install_name_tool -change "/Qt/Qt5.3.0/5.3/clang_64/lib/QtCore.framework/Versions/5/QtCore"       "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore" ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrexport
install_name_tool -change "/Qt/Qt5.3.0/5.3/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork" "@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork" ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrexport
install_name_tool -change "/Qt/Qt5.3.0/5.3/clang_64/lib/QtXml.framework/Versions/5/QtXml"         "@executable_path/../Frameworks/QtXml.framework/Versions/5/QtXml" ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrexport

install_name_tool -change "/Qt/Qt5.3.0/5.3/clang_64/lib/QtCore.framework/Versions/5/QtCore"       "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore" ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrbridge
install_name_tool -change "/Qt/Qt5.3.0/5.3/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork" "@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork" ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrbridge

install_name_tool -change "/Qt/Qt5.3.0/5.3/clang_64/lib/QtCore.framework/Versions/5/QtCore"       "@executable_path/../Frameworks/QtCore.framework/Versions/5/QtCore" ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrdeamon
install_name_tool -change "/Qt/Qt5.3.0/5.3/clang_64/lib/QtNetwork.framework/Versions/5/QtNetwork" "@executable_path/../Frameworks/QtNetwork.framework/Versions/5/QtNetwork" ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrdeamon
install_name_tool -change "/Qt/Qt5.3.0/5.3/clang_64/lib/QtXml.framework/Versions/5/QtXml"         "@executable_path/../Frameworks/QtXml.framework/Versions/5/QtXml" ~/Desktop/release/Gideros\ Studio.app/Contents/Tools/gdrdeamon

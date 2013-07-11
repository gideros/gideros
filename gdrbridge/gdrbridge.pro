#-------------------------------------------------
#
# Project created by QtCreator 2012-08-07T16:11:44
#
#-------------------------------------------------

QT       += core network

QT       -= gui

TARGET = gdrbridge
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += main.cpp

macx {
QMAKE_POST_LINK += install_name_tool -change "/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/lib/QtCore.framework/Versions/4/QtCore" "@executable_path/../Frameworks/QtCore.framework/Versions/4/QtCore" $(TARGET);
QMAKE_POST_LINK += install_name_tool -change "/Qt/1.2.1/Desktop/Qt/4.8.1/gcc/lib/QtNetwork.framework/Versions/4/QtNetwork" "@executable_path/../Frameworks/QtNetwork.framework/Versions/4/QtNetwork" $(TARGET);
}

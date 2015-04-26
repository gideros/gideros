cd ../build
cd QScintilla-gpl-2.9/Qt4Qt5
sed -i.bak '34 i\
#define USING_OSX_KEYS' qscicommandset.cpp
$QT/bin/qmake qscintilla.pro
make
make install
cd ../..
cd ../scripts

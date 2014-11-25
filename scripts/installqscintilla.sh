cd ../tmp
cd QScintilla-gpl-2.8.4/Qt4Qt5
sed -i.bak '34 i\
#define USING_OSX_KEYS' qscicommandset.cpp
$QT/bin/qmake qscintilla.pro
make
make install
cd ../..
cd ../scripts

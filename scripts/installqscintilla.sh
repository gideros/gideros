rm QScintilla-gpl-2.8.4.tar.gz
wget http://sourceforge.net/projects/pyqt/files/QScintilla2/QScintilla-2.8.4/QScintilla-gpl-2.8.4.tar.gz

rm -rf QScintilla-gpl-2.8.4
tar zxvf QScintilla-gpl-2.8.4.tar.gz

cd QScintilla-gpl-2.8.4/Qt4Qt5
$QT/bin/qmake qscintilla.pro
make
make install

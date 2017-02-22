
#export QT=/Users/yizheng/Qt5.6.0/5.6/clang_64
#export QT_WIN=~/.wine/drive_c/Qt/Qt5.6.0/5.6/mingw49_32
#export QT_DLL=54
export IOS_SDK=10.2
export TVOS_SDK=10.1
export GVERSION=2016.12.1

cd scripts
#echo 'Updating api annotation'
#wget "http://docs.giderosmobile.com/reference/autocomplete.php" -O ../ui/Resources/gideros_annot.api

#echo 'updating docs'
#bash docs-update.sh


echo 'Building Qt applications for Windows...'
rm -rf ../Sdk
rm -rf ../gdrdeamon/qtsinglecoreapplication.o
wine cmd /c qt5\\buildqtlibs.bat > /dev/null
wine cmd /c qt5\\buildplugins.bat > /dev/null
wine cmd /c qt5\\cleanqt.bat > /dev/null
wine cmd /c qt5\\buildqt.bat > /dev/null

echo 'Copying Windows files...'
bash copywin.sh > /dev/null

echo 'Creating Windows installation package...'
bash createwinpackage.sh > /dev/null

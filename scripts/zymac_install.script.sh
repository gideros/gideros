
#export QT=/Users/yizheng/Qt5.6.0/5.6/clang_64
#export QT_WIN=~/.wine/drive_c/Qt/Qt5.6.0/5.6/mingw49_32
#export QT_DLL=54
export IOS_SDK=10.2
export TVOS_SDK=10.1
export GVERSION=2016.12.1

rm -rf build
mkdir build

cd scripts
#echo 'Updating api annotation'
#wget "http://docs.giderosmobile.com/reference/autocomplete.php" -O ../ui/Resources/gideros_annot.api

#echo 'updating docs'
#bash docs-update.sh


echo 'Building Qt applications for Mac...'
rm -rf ../Sdk
bash qt5/buildqtlibs.sh > /dev/null
bash qt5/buildplugins.sh > /dev/null
bash qt5/cleanqt.sh > /dev/null
bash qt5/buildqt.sh > /dev/null

echo 'Building iOS libraries...'
bash cleanioslibs.sh > /dev/null
bash buildioslibs.sh > /dev/null
bash buildiosplugins.sh > /dev/null
bash cleanioslibs.sh > /dev/null
sleep 200
bash cleanatvlibs.sh > /dev/null
bash buildatvlibs.sh > /dev/null
bash buildatvplugins.sh > /dev/null

echo 'Building Android libraries...'
bash makejar.sh > /dev/null
bash buildandroidlibs.sh > /dev/null
bash buildandroidso.sh > /dev/null
bash buildandroidplugins.sh > /dev/null



echo 'Copying Windows files...'
bash copywin.sh > /dev/null

echo 'Copying Mac files...'
bash copymac.sh


#echo 'Creating Windows installation package...'
#bash createwinpackage.sh > /dev/null

echo 'Creating Mac installation package...'
bash createmacpackage.sh





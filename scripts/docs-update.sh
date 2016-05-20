cd ..

#download new docs
wget --recursive --no-clobber --page-requisites --html-extension --convert-links --restrict-file-names=windows --domains docs.giderosmobile.com --no-parent http://docs.giderosmobile.com/

#remove old docs
rm -rf doc

#place new docs
mv -f docs.giderosmobile.com/ doc

cd scripts

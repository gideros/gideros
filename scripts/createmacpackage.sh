cd ..

mkdir build/Gideros\ 2014.04
mv build/mac build/Gideros\ 2014.04/Gideros\ Studio
ln -s /Applications build/Gideros\ 2014.04/Applications

rm build/gideros_2014.04.dmg
hdiutil create build/gideros_2014.04.dmg -srcfolder build/Gideros\ 2014.04

cd scripts

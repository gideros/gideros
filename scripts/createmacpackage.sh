cd ..

mkdir build/Gideros\ 2014.10
mv build/mac build/Gideros\ 2014.10/Gideros\ Studio
ln -s /Applications build/Gideros\ 2014.10/Applications

rm build/gideros_2014.10.dmg
hdiutil create build/gideros_2014.10.dmg -srcfolder build/Gideros\ 2014.10

cd scripts

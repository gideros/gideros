cd ..

mkdir build/Gideros
mv build/mac build/Gideros/Gideros\ Studio
ln -s /Applications build/Gideros/Applications

rm build/gideros.dmg
hdiutil create build/gideros.dmg -srcfolder build/Gideros

cd scripts

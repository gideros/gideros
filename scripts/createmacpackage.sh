cd ..

mkdir build/Gideros
mv build/mac build/Gideros/Gideros\ Studio
ln -s /Applications build/Gideros/Applications

rm build/gideros_$GVERSION.dmg
hdiutil create build/gideros_$GVERSION.dmg -srcfolder build/Gideros

cd scripts

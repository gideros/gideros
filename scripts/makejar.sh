cd ..
cd android
cd GiderosAndroidPlayer
rm -rf src/com/android
rm -rf src/com/giderosmobile/android/plugins
mkdir -p libs
cp lib/*.jar libs
ant clean
ant debug
rm gideros.jar
cd bin
cd classes
rm ./com/giderosmobile/android/player/GiderosAndroidPlayerActivity.class
rm ./com/giderosmobile/android/player/GiderosGLSurfaceView.class
rm ./com/giderosmobile/android/player/GiderosRenderer.class
rm ./com/giderosmobile/android/player/R.class
rm ./com/giderosmobile/android/player/R\$*.class
jar cvf gideros.jar com
mv gideros.jar ../..
cd ..
cd ..
cd ..
cd ..
cd scripts

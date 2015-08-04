cd ..
cd android
cd GiderosAndroidPlayer
rm -rf src/com/android
rm -rf src/com/giderosmobile/android/plugins
call ant clean
call ant debug
rm gideros.jar
cd bin
cd classes
rem rm -rf ./.svn
rem rm -rf ./com/.svn
rem rm -rf ./com/giderosmobile/.svn
rem rm -rf ./com/giderosmobile/android/.svn
rem rm -rf ./com/giderosmobile/android/player/.svn
rem rm -rf ./com/loopj/.svn
rem rm -rf ./com/loopj/android/.svn
rem rm -rf ./com/loopj/android/http/.svn
rm ./com/giderosmobile/android/player/GiderosAndroidPlayerActivity.class
rm ./com/giderosmobile/android/player/GiderosGLSurfaceView.class
rm ./com/giderosmobile/android/player/GiderosRenderer.class
rm ./com/giderosmobile/android/player/R.class
rm ./com/giderosmobile/android/player/R$*.class
jar.exe cvf gideros.jar com
mv gideros.jar ..\..
cd ..
cd ..
cd ..
cd ..
cd scripts

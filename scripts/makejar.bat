cd ..
cd android
cd GiderosAndroidPlayer
rmdir /S /Q src\com\android
rmdir /S /Q src\com\giderosmobile\android\plugins
if not exists libs mkdir libs
cp lib/*.jar libs
call ant clean
call ant debug
del gideros.jar
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
del .\com\giderosmobile\android\player\GiderosAndroidPlayerActivity.class
del .\com\giderosmobile\android\player\GiderosGLSurfaceView.class
del .\com\giderosmobile\android\player\GiderosRenderer.class
del .\com\giderosmobile\android\player\R.class
del .\com\giderosmobile\android\player\R$*.class
jar.exe cvf gideros.jar com
move gideros.jar ..\..
cd ..
cd ..
cd ..
cd ..
cd scripts

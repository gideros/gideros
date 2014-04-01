cd ..
cd plugins

cd BitOp\source
rm -rf libs obj
call c:\android-ndk-r9d\ndk-build
cd ..\..

cd LuaSocket\source
rm -rf libs obj
call c:\android-ndk-r9d\ndk-build
cd ..\..

cd LPeg\source
rm -rf libs obj
call c:\android-ndk-r9d\ndk-build
cd ..\..

cd LuaFileSystem\source
rm -rf libs obj
call c:\android-ndk-r9d\ndk-build
cd ..\..

cd "Google Billing\source\Android"
rm -rf libs obj
call c:\android-ndk-r9d\ndk-build
cd ..\..\..

cd LuaSQLite3\source
rm -rf libs obj
call c:\android-ndk-r9d\ndk-build
cd ..\..

cd Microphone\source\Android
rm -rf libs obj
call c:\android-ndk-r9d\ndk-build
cd ..\..\..

cd Flurry\source\Android
rm -rf libs obj
call c:\android-ndk-r9d\ndk-build
cd ..\..\..

cd Facebook\source\Android
rm -rf libs obj
call c:\android-ndk-r9d\ndk-build
cd ..\..\..

cd JSON\source
rm -rf libs obj
call c:\android-ndk-r9d\ndk-build
cd ..\..

cd ..
cd scripts

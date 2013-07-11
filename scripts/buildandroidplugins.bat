cd ..
cd plugins

cd BitOp
cd source
rm -rf libs obj
call C:\android-ndk-r8e\ndk-build
cd ..\..

cd LuaSocket
cd source
rm -rf libs obj
call C:\android-ndk-r8e\ndk-build
cd ..\..

cd LPeg
cd source
rm -rf libs obj
call C:\android-ndk-r8e\ndk-build
cd ..\..

cd LuaFileSystem
cd source
rm -rf libs obj
call C:\android-ndk-r8e\ndk-build
cd ..\..

cd "Google Billing"
cd source
cd Android
rm -rf libs obj
call C:\android-ndk-r8e\ndk-build
cd ..\..\..

cd LuaSQLite3
cd source
rm -rf libs obj
call C:\android-ndk-r8e\ndk-build
cd ..\..

cd Microphone
cd source
rm -rf libs obj
call C:\android-ndk-r8e\ndk-build
cd ..\..

cd ..
cd scripts


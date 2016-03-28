cd ..
cd android
rmdir /S /Q build
mkdir build
mkdir build\jni
xcopy /S /I lib\jni\* build\jni
cd build

call c:\android-ndk\android-ndk-r11b\ndk-build

rmdir /S /Q ..\..\Sdk\lib\android
mkdir ..\..\Sdk\lib\android
xcopy /S /I libs\* ..\..\Sdk\lib\android 

cd ..
cd ..
cd scripts


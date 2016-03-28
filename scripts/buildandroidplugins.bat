cd ..
cd plugins

cd bitop\source
rmdir /S /Q libs
rmdir /S /Q obj
call c:\android-ndk\android-ndk-r11b\ndk-build
cd ..\..

cd luasocket\source
rmdir /S /Q libs
rmdir /S /Q obj
call c:\android-ndk\android-ndk-r11b\ndk-build
cd ..\..

cd lpeg\source
rmdir /S /Q libs
rmdir /S /Q obj
call c:\android-ndk\android-ndk-r11b\ndk-build
cd ..\..

cd lfs\source
rmdir /S /Q libs
rmdir /S /Q obj
call c:\android-ndk\android-ndk-r11b\ndk-build
cd ..\..

cd "googlebilling\source\Android"
rmdir /S /Q libs
rmdir /S /Q obj
call c:\android-ndk\android-ndk-r11b\ndk-build
cd ..\..\..

cd luasqlite3\source
rmdir /S /Q libs
rmdir /S /Q obj
call c:\android-ndk\android-ndk-r11b\ndk-build
cd ..\..

cd microphone\source\Android
rmdir /S /Q libs
rmdir /S /Q obj
call c:\android-ndk\android-ndk-r11b\ndk-build
cd ..\..\..

cd flurry\source\Android
rmdir /S /Q libs
rmdir /S /Q obj
call c:\android-ndk\android-ndk-r11b\ndk-build
cd ..\..\..

cd facebook\source\Android
rmdir /S /Q libs
rmdir /S /Q obj
call c:\android-ndk\android-ndk-r11b\ndk-build
cd ..\..\..

cd json\source
rmdir /S /Q libs
rmdir /S /Q obj
call c:\android-ndk\android-ndk-r11b\ndk-build
cd ..\..

cd ..
cd scripts

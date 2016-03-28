cd ..

cd libgvfs
rmdir /S /Q libs
rmdir /S /Q obj
call c:\android-ndk\android-ndk-r11b\ndk-build
cd ..

cd lua
rmdir /S /Q libs
rmdir /S /Q obj
call c:\android-ndk\android-ndk-r11b\ndk-build
cd ..

cd scripts

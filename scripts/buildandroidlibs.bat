cd ..

cd libgid\external\zlib-1.2.8
rm -rf libs obj
call c:\android-ndk-r9b\ndk-build
cd ..\..\..

cd libgvfs
rm -rf libs obj
call c:\android-ndk-r9b\ndk-build
cd ..

cd lua
rm -rf libs obj
call c:\android-ndk-r9b\ndk-build
cd ..

cd scripts

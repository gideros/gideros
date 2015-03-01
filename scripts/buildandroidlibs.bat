cd ..

cd libgvfs
rm -rf libs obj
call c:\android-ndk-r10d\ndk-build
cd ..

cd lua
rm -rf libs obj
call c:\android-ndk-r10d\ndk-build
cd ..

cd scripts

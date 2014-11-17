cd ..

cd libgvfs
rm -rf libs obj
$ANDROID_NDK/ndk-build
cd ..

cd lua
rm -rf libs obj
$ANDROID_NDK/ndk-build
cd ..

cd scripts

cd ..
cd android
rm -rf build
mkdir build
mkdir build/jni
cp -R lib/jni/* build/jni
cd build

$ANDROID_NDK/ndk-build

rm -rf ../../Sdk/lib/android
mkdir ../../Sdk/lib/android
cp -R libs/* ../../Sdk/lib/android 

cd ..
cd ..
cd scripts


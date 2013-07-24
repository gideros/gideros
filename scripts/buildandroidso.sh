cd /Volumes/gideros/scripts

cd ..
cd android
rm -rf build
mkdir build
mkdir build/jni
cp -R lib/jni/* build/jni
cd build


rm -rf libs2
mkdir libs2
mkdir libs2/armeabi
mkdir libs2/armeabi-v7a

cp jni/Application-arm6.mk jni/Application.mk
/android-ndk-r6-crystax-2/ndk-build
cp libs/armeabi/libgideros.so libs2/armeabi

cp jni/Application-arm7.mk jni/Application.mk
/android-ndk-r6-crystax-2/ndk-build
cp libs/armeabi-v7a/libgideros.so libs2/armeabi-v7a

rm -rf libs
mv libs2 libs

cd ..
cd ..
cd scripts

cd ~/myprojects/gideros/scripts
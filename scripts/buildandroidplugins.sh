cd ..
cd plugins

for d in *; do
cd $d/source
if [ -d Android ] || [ -d jni ] ; then
if [ -d Android ]; then
cd Android
fi
rm -rf libs obj
$ANDROID_NDK/ndk-build
fi
cd ..
if [ -d Android ]; then
cd ..
fi
cd ..
done

cd ../scripts

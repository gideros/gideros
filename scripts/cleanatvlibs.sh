cd ..

#cd libgid/external/zlib-1.2.8
#rm -rf build/Release-iphoneos
#rm -rf build/Release-iphonesimulator
#rm -rf build/zlib.build
#cd ../../..

cd libgvfs
rm -rf build/Release-appletvos
rm -rf build/Release-appletvsimulator
rm -rf build/gvfs.build
cd ..

cd lua
rm -rf build/Release-appletvos
rm -rf build/Release-appletvsimulator
rm -rf build/lua.build
cd ..

cd ios/iosplayer
rm -rf build/Release-appletvos
rm -rf build/Release-appletvsimulator
rm -rf build/iosplayer.build
cd ../..

cd plugins
cd LuaSocket
cd source
rm -rf build/Release-appletvos
rm -rf build/Release-appletvsimulator
rm -rf build/luasocket.build
rm -rf libluasocket.a
cd ../../..

cd scripts
cd ..
cd plugins
cd LuaSocket
cd source
rm -rf build
xcodebuild -project luasocket.xcodeproj -alltargets -sdk iphonesimulator7.1 -configuration Release
xcodebuild -project luasocket.xcodeproj -alltargets -sdk iphoneos7.1 -configuration Release
lipo build/Release-iphoneos/libluasocket.a build/Release-iphonesimulator/libluasocket.a -create -output libluasocket.a
cd ../../..
cd scripts


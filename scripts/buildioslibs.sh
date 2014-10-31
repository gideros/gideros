cd ..


#cd libgid/external/zlib-1.2.8
#xcodebuild -alltargets -sdk iphonesimulator7.1 -configuration Release -project zlib.xcodeproj
#xcodebuild -alltargets -sdk iphoneos7.1 -configuration Release -project zlib.xcodeproj
#lipo build/Release-iphoneos/libzlib.a build/Release-iphonesimulator/libzlib.a -create -output libzlib.a
#cd ../../..

cd libgvfs
xcodebuild -alltargets -sdk iphonesimulator$IOS_SDK -configuration Release -project gvfs.xcodeproj
xcodebuild -alltargets -sdk iphoneos$IOS_SDK -configuration Release -project gvfs.xcodeproj
lipo build/Release-iphoneos/libgvfs.a build/Release-iphonesimulator/libgvfs.a -create -output libgvfs.a
cd ..

cd lua
xcodebuild -alltargets -sdk iphonesimulator$IOS_SDK -configuration Release -project lua.xcodeproj
xcodebuild -alltargets -sdk iphoneos$IOS_SDK -configuration Release -project lua.xcodeproj
lipo build/Release-iphoneos/liblua.a build/Release-iphonesimulator/liblua.a -create -output liblua.a
cd ..

cd ios/iosplayer
xcodebuild -alltargets -sdk iphonesimulator$IOS_SDK -configuration Release -project iosplayer.xcodeproj
xcodebuild -alltargets -sdk iphoneos$IOS_SDK -configuration Release -project iosplayer.xcodeproj
lipo build/Release-iphoneos/libiosplayer.a build/Release-iphonesimulator/libiosplayer.a -create -output libgideros.a
cd ../..


cd scripts

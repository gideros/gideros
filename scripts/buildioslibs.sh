cd ..


cd libgvfs
xcodebuild -alltargets -sdk iphonesimulator7.0 -configuration Release -project gvfs.xcodeproj
xcodebuild -alltargets -sdk iphoneos7.0 -configuration Release -project gvfs.xcodeproj
lipo build/Release-iphoneos/libgvfs.a build/Release-iphonesimulator/libgvfs.a -create -output libgvfs.a
cd ..

cd lua
xcodebuild -alltargets -sdk iphonesimulator7.0 -configuration Release -project lua.xcodeproj
xcodebuild -alltargets -sdk iphoneos7.0 -configuration Release -project lua.xcodeproj
lipo build/Release-iphoneos/liblua.a build/Release-iphonesimulator/liblua.a -create -output liblua.a
cd ..

cd ios
cd iosplayer
xcodebuild -alltargets -sdk iphonesimulator7.0 -configuration Release -project iosplayer.xcodeproj
xcodebuild -alltargets -sdk iphoneos7.0 -configuration Release -project iosplayer.xcodeproj
lipo build/Release-iphoneos/libiosplayer.a build/Release-iphonesimulator/libiosplayer.a -create -output libgideros.a
cd ..
cd ..


cd scripts

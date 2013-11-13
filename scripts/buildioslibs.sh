cd ..


cd libgvfs
xcodebuild -alltargets -sdk iphonesimulator7.0 -configuration Release -project gvfs.xcodeproj
xcodebuild -alltargets -sdk iphoneos7.0 -configuration Release -project gvfs.xcodeproj
cd ..

cd lua
xcodebuild -alltargets -sdk iphonesimulator7.0 -configuration Release -project lua.xcodeproj
xcodebuild -alltargets -sdk iphoneos7.0 -configuration Release -project lua.xcodeproj
cd ..

cd ios
cd iosplayer
xcodebuild -alltargets -sdk iphonesimulator7.0 -configuration Release -project iosplayer.xcodeproj
xcodebuild -alltargets -sdk iphoneos7.0 -configuration Release -project iosplayer.xcodeproj
cd ..
cd ..


cd scripts

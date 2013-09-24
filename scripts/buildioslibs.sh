cd ..
cd ios
cd iosplayer
xcodebuild -project iosplayer.xcodeproj -alltargets -sdk iphonesimulator6.1 -configuration Release
xcodebuild -project iosplayer.xcodeproj -alltargets -sdk iphoneos6.1 -configuration Release
cd ..
cd ..
cd scripts

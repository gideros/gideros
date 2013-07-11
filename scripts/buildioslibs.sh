cd ..
cd ios
cd iosplayer
xcodebuild -project iosplayer.xcodeproj -alltargets -sdk iphonesimulator5.1 -configuration Release
xcodebuild -project iosplayer.xcodeproj -alltargets -sdk iphoneos5.1 -configuration Release
cd ..
cd ..
cd scripts

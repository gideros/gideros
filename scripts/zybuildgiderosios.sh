cd ios/iosplayer
xcodebuild -alltargets -sdk iphonesimulator$IOS_SDK -configuration Release -project iosplayer.xcodeproj
xcodebuild -alltargets -sdk iphoneos$IOS_SDK -configuration Release -project iosplayer.xcodeproj
lipo build/Release-iphoneos/libiosplayer.a build/Release-iphonesimulator/libiosplayer.a -create -output libgideros.a
mv libgideros.a ../../ui/Templates/Xcode4/iOS\ Template/iOS\ Template
cd ../..

cp ios/iosplayer/iosplayer/giderosapi.h ui/Templates/Xcode4/iOS\ Template/iOS\ Template
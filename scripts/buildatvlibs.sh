cd ..


#cd libgid/external/zlib-1.2.8
#xcodebuild -alltargets -sdk iphonesimulator7.1 -configuration Release -project zlib.xcodeproj
#xcodebuild -alltargets -sdk iphoneos7.1 -configuration Release -project zlib.xcodeproj
#lipo build/Release-iphoneos/libzlib.a build/Release-iphonesimulator/libzlib.a -create -output libzlib.a
#cd ../../..

cd libgvfs
/Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -alltargets -sdk appletvos$TVOS_SDK -configuration Release -project gvfs.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode"
/Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -alltargets -sdk appletvsimulator -configuration Release -project gvfs.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1'
lipo build/Release-appletvsimulator/libgvfs.a build/Release-appletvos/libgvfs.a -create -output libgvfs.a
mv libgvfs.a ../ui/Templates/Xcode4/iOS\ Template/AppleTV
cd ..

cd lua
/Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -alltargets -sdk appletvos$TVOS_SDK -configuration Release -project lua.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode"
/Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -alltargets -sdk appletvsimulator -configuration Release -project lua.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1'
lipo build/Release-appletvsimulator/liblua.a build/Release-appletvos/liblua.a -create -output liblua.a
mv liblua.a ../ui/Templates/Xcode4/iOS\ Template/AppleTV
cd ..

cd ios/iosplayer
/Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -alltargets -sdk appletvos$TVOS_SDK -configuration Release -project iosplayer.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1' OTHER_CFLAGS="-fembed-bitcode"
/Applications/Xcode.app/Contents/Developer/usr/bin/xcodebuild -alltargets -sdk appletvsimulator -configuration Release -project iosplayer.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1'
lipo build/Release-appletvsimulator/libiosplayer.a build/Release-appletvos/libiosplayer.a -create -output libgideros.a
mv libgideros.a ../../ui/Templates/Xcode4/iOS\ Template/AppleTV
cd ../..

cp ios/iosplayer/iosplayer/giderosapi.h ui/Templates/Xcode4/iOS\ Template/AppleTV

cd scripts

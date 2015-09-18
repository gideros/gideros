cd ..


#cd libgid/external/zlib-1.2.8
#xcodebuild -alltargets -sdk iphonesimulator7.1 -configuration Release -project zlib.xcodeproj
#xcodebuild -alltargets -sdk iphoneos7.1 -configuration Release -project zlib.xcodeproj
#lipo build/Release-iphoneos/libzlib.a build/Release-iphonesimulator/libzlib.a -create -output libzlib.a
#cd ../../..

cd libgvfs
/Applications/Xcode-beta.app/Contents/Developer/usr/bin/xcodebuild -alltargets -sdk appletvos9.0 -configuration Release -project gvfs.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1'
/Applications/Xcode-beta.app/Contents/Developer/usr/bin/xcodebuild -alltargets -sdk appletvsimulator -configuration Release -project gvfs.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1'
lipo build/Release-appletvsimulator/libgvfs.a build/Release-appletvos/libgvfs.a -create -output libgvfs_tv.a
mv libgvfs_tv.a ../ui/Templates/Xcode4/iOS\ Template/iOS\ Template
cd ..

cd lua
/Applications/Xcode-beta.app/Contents/Developer/usr/bin/xcodebuild -alltargets -sdk appletvos9.0 -configuration Release -project lua.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1'
/Applications/Xcode-beta.app/Contents/Developer/usr/bin/xcodebuild -alltargets -sdk appletvsimulator -configuration Release -project lua.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1'
lipo build/Release-appletvsimulator/liblua.a build/Release-appletvos/liblua.a -create -output liblua_tv.a
mv liblua_tv.a ../ui/Templates/Xcode4/iOS\ Template/iOS\ Template
cd ..

cd ios/iosplayer
/Applications/Xcode-beta.app/Contents/Developer/usr/bin/xcodebuild -alltargets -sdk appletvos9.0 -configuration Release -project iosplayer.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1'
/Applications/Xcode-beta.app/Contents/Developer/usr/bin/xcodebuild -alltargets -sdk appletvsimulator -configuration Release -project iosplayer.xcodeproj GCC_PREPROCESSOR_DEFINITIONS='${inherited} TARGET_OS_TV=1'
lipo build/Release-appletvsimulator/libiosplayer.a build/Release-appletvos/libiosplayer.a -create -output libgideros_tv.a
mv libgideros_tv.a ../../ui/Templates/Xcode4/iOS\ Template/iOS\ Template
cd ../..

cd scripts

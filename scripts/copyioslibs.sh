lipo ../ios/iosplayer/build/Release-iphoneos/libiosplayer.a ../ios/iosplayer/build/Release-iphonesimulator/libiosplayer.a -create -output libgideros.a

cp libgideros.a                                              ~/myprojects/gideros/ios/GiderosiOSPlayer/GiderosiOSPlayer
cp ~/myprojects/gideros/ios/iosplayer/iosplayer/giderosapi.h ~/myprojects/gideros/ios/GiderosiOSPlayer/GiderosiOSPlayer
cp libgideros.a                                              ~/myprojects/gideros/ui/Templates/Xcode4/iOS\ Template/iOS\ Template
cp ~/myprojects/gideros/ios/iosplayer/iosplayer/giderosapi.h ~/myprojects/gideros/ui/Templates/Xcode4/iOS\ Template/iOS\ Template


cp libgideros.a                                              /Volumes/gideros/ios/GiderosiOSPlayer/GiderosiOSPlayer
cp ~/myprojects/gideros/ios/iosplayer/iosplayer/giderosapi.h /Volumes/gideros/ios/GiderosiOSPlayer/GiderosiOSPlayer
cp libgideros.a                                              /Volumes/gideros/ui/Templates/Xcode4/iOS\ Template/iOS\ Template
cp ~/myprojects/gideros/ios/iosplayer/iosplayer/giderosapi.h /Volumes/gideros/ui/Templates/Xcode4/iOS\ Template/iOS\ Template

rm libgideros.a


lipo ../libgvfs/build/Release-iphoneos/libgvfs.a ../libgvfs/build/Release-iphonesimulator/libgvfs.a -create -output libgvfs.a
cp libgvfs.a ~/myprojects/gideros/ios/GiderosiOSPlayer/GiderosiOSPlayer
cp libgvfs.a ~/myprojects/gideros/ui/Templates/Xcode4/iOS\ Template/iOS\ Template
cp libgvfs.a /Volumes/gideros/ios/GiderosiOSPlayer/GiderosiOSPlayer
cp libgvfs.a /Volumes/gideros/ui/Templates/Xcode4/iOS\ Template/iOS\ Template
rm libgvfs.a


lipo ../lua/build/Release-iphoneos/liblua.a ../lua/build/Release-iphonesimulator/liblua.a -create -output liblua.a
cp liblua.a ~/myprojects/gideros/ios/GiderosiOSPlayer/GiderosiOSPlayer
cp liblua.a ~/myprojects/gideros/ui/Templates/Xcode4/iOS\ Template/iOS\ Template
cp liblua.a /Volumes/gideros/ios/GiderosiOSPlayer/GiderosiOSPlayer
cp liblua.a /Volumes/gideros/ui/Templates/Xcode4/iOS\ Template/iOS\ Template
rm liblua.a

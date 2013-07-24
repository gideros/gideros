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

#cd /Applications/Xcode/5.0.2/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin
#ln -s clang gcc
#ln -s clang++ g++

#add src/Makefile
#ifeq (iOS,$(TARGET_SYS))
#  BUILDMODE= static
#endif

IXCODE=`xcode-select -print-path`
ISDK=$IXCODE/Platforms/iPhoneOS.platform/Developer
ISDKVER=iPhoneOS7.0.sdk
ISDKP=$IXCODE/Toolchains/XcodeDefault.xctoolchain/usr/bin/
ISDKF="-arch armv7 -isysroot $ISDK/SDKs/$ISDKVER -miphoneos-version-min=5.0"
make clean
make HOST_CC="gcc -m32 -arch i386" CROSS=$ISDKP TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS TARGET_CFLAGS="-I../../libgvfs" TARGET_LIBS="-L../../libgvfs -lgvfs"
cp src/libluajit.a libluajit-armv7.a

IXCODE=`xcode-select -print-path`
ISDK=$IXCODE/Platforms/iPhoneOS.platform/Developer
ISDKVER=iPhoneOS7.0.sdk
ISDKP=$IXCODE/Toolchains/XcodeDefault.xctoolchain/usr/bin/
ISDKF="-arch armv7s -isysroot $ISDK/SDKs/$ISDKVER -miphoneos-version-min=5.0"
make clean
make HOST_CC="gcc -m32 -arch i386" CROSS=$ISDKP TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS TARGET_CFLAGS="-I../../libgvfs" TARGET_LIBS="-L../../libgvfs -lgvfs"
cp src/libluajit.a libluajit-armv7s.a

IXCODE=`xcode-select -print-path`
ISDK=$IXCODE/Platforms/iPhoneSimulator.platform/Developer
ISDKVER=iPhoneSimulator7.0.sdk
ISDKP=$IXCODE/Toolchains/XcodeDefault.xctoolchain/usr/bin/
ISDKF="-arch i386 -isysroot $ISDK/SDKs/$ISDKVER -miphoneos-version-min=5.0"
make clean
make HOST_CC="gcc -m32 -arch i386" CROSS=$ISDKP TARGET_FLAGS="$ISDKF" TARGET_SYS=iOS TARGET_CFLAGS="-I../../libgvfs" TARGET_LIBS="-L../../libgvfs -lgvfs"
cp src/libluajit.a libluajit-i386.a

lipo libluajit-armv7.a libluajit-armv7s.a libluajit-i386.a -create -output libluajit.a

make clean
rm libluajit-armv7.a
rm libluajit-armv7s.a
rm libluajit-i386.a

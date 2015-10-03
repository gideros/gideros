rm -rf ../../release/*
mkdir ../../release

cp ../libgid/libgid.so* ../../release
cp ../libgvfs/libgvfs.so* ../../release
cp ../lua/liblua.so* ../../release
cp ../libgideros/libgideros.so* ../../release
cp ../libpystring/libpystring.so* ../../release
cp ../libgid/external/openal-soft-1.13/build/gcc463_pi/libopenal.so* ../../release

cp ../ui/GiderosStudio ../../release
cp ../player/GiderosPlayer ../../release
cp ../texturepacker/GiderosTexturePacker ../../release
cp ../fontcreator/GiderosFontCreator ../../release

cp -R ../ui/Resources ../../release/Resources
cp -R ../ui/Tools ../../release/Tools
cp -R ../samplecode ../../release/Examples

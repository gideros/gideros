:: Does not work in win32 as needs QObject, QCameraInfo from Qt

g++ -I ../../libgideros -I ../../libgvfs ^
    -I ../../lua/src -I ../../libgid/include ^
    -I ./libng -I ./jpeglib ^
-c ../../plugins/gmedia/source/*.cpp

g++ -o gmedia.dll -shared media.o mediabinder.o ../lua.dll ../gid.dll ../gideros.dll ../gvfs.dll ^
-L"../../libgid/external/libpng-1.6.2/build/mingw48_32" -lpng ^
-L"../../libgid/external/jpeg-9/build/mingw48_32" -ljpeg ^
-L"../../libgid/external/zlib-1.2.8/build/mingw48_32" -lzlibx

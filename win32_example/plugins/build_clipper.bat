g++ -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/clipper/source/*.cpp 

g++ -o clipper.dll -shared clipper.o clipperbinder.o ../lua.dll ../gid.dll ../gideros.dll

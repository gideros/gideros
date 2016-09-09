gcc -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include -I../../plugins/controller/source ^
-DGID_LIBRARY ^
-c ../../plugins/controller/source/gamepad/Gamepad_private.c ^
../../plugins/controller/source/gamepad/Gamepad_windows_mm.c

g++ -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include -I../../plugins/controller/source ^
-DGID_LIBRARY -DWIN32_NOQT ^
-c ../../plugins/controller/source/*.cpp 

g++ -o controller.dll -shared ^
controllerbinder.o controller.o Gamepad_private.o gcontroller.o Gamepad_windows_mm.o ^
-lwinmm ../lua.dll ../gid.dll ../gideros.dll

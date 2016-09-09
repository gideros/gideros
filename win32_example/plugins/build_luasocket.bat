:: ---------
:: LUASOCKET
:: ---------

gcc -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-D_WIN32_WINNT=0x0501 -DLUA_NOCOMPAT_MODULE -DLUASOCKET_INET_PTON ^
-c ../../plugins/luasocket/source/*.c

g++ -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-DLUA_NOCOMPAT_MODULE -DLUASOCKET_INET_PTON ^
-c ../../plugins/luasocket/source/*.cpp 

g++ -o luasocket.dll -shared ^
auxiliar.o buffer.o except.o inet.o io.o luasocket.o options.o select.o ^
tcp.o timeout.o udp.o mime.o luasocket_stub.o wsocket.o ^
-lws2_32 ../lua.dll

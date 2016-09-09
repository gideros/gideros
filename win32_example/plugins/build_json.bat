:: ----
:: JSON
:: ----

gcc -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/json/source/*.c

g++ -I ../../libgideros -I ../../libgvfs -I ../../lua/src -I ../../libgid/include ^
-c ../../plugins/json/source/*.cpp

g++ -o json.dll -shared strbuf.o fpconv.o lua_cjson.o lua_cjson_stub.o ../lua.dll

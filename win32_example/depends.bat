set include_paths= ^
-I..\luabinding ^
-I..\libnetwork ^
-I..\libgid\external\glew-1.10.0\include ^
-I..\libgid\external\freetype-2.4.12\src ^
-I..\libgid\external\freetype-2.4.12\include ^
-I..\libgid\external\mpg123-1.15.3\src ^
-I..\libgid\external\pthreads-w32-2-9-1-release\Pre-built.2\include ^
-I..\external\liquidfun-1.0.0\liquidfun\Box2D ^
-I..\libgid\include\winrt ^
-I..\libgid\external\snappy-1.1.0 ^
-I..\libgid\external\libpng-1.6.2 ^
-I..\libgid\external\jpeg-9 ^
-I..\libgid\external\openal-soft-1.13\include\AL ^
-I..\libgid\external\mpg123-1.15.3\src\libmpg123 ^
-I..\lua\src ^
-I..\libgid\external\zlib-1.2.8 ^
-I..\external\glu ^
-I..\libpystring ^
-I..\libgvfs ^
-I..\libgideros ^
-I..\2dsg ^
-I..\libgid\include

set preprocessor= ^
-DPTW32_STATIC_LIB ^
-DOPT_GENERIC ^
-DREAL_IS_FLOAT ^
-DFT2_BUILD_LIBRARY ^
-DPYSTRING_LIBRARY

g++ %include_paths% -MM win32.cpp > win32.dep

g++ %include_paths% -U_WIN32 -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Common\*.cpp > box2d1.dep
g++ %include_paths% -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Collision\*.cpp > box2d2.dep
g++ %include_paths% -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Collision\Shapes\*.cpp > box2d3.dep
g++ %include_paths% -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Dynamics\*.cpp > box2d4.dep
g++ %include_paths% -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Dynamics\Contacts\*.cpp > box2d5.dep
g++ %include_paths% -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Dynamics\Joints\*.cpp > box2d6.dep
g++ %include_paths% -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Rope\*.cpp > box2d7.dep
g++ %include_paths% -MM ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Particle\*.cpp > box2d8.dep

g++ %include_paths% -MM ..\2dsg\*.cpp > 2dsg.dep

g++ %include_paths% -MM ..\luabinding\*.cpp  > luabinding.dep
g++ %include_paths% -MM ..\luabinding\tlsf.c > tlsf.dep

g++ %include_paths% -MM ..\libnetwork\*.cpp > libnetwork.dep

g++ %include_paths% -MM ..\libpvrt\*.cpp > libpvrt.dep
g++ %include_paths% -MM ..\external\glu\libtess\*.c > libtess.dep

g++ %include_paths% -MM ..\libgid\src\md5.c > md5.dep
g++ %include_paths% -MM ..\libgid\src\utf8.c > utf8.dep

g++ %include_paths% -MM ..\libgid\src\*.cpp > libgid.dep
g++ %include_paths% -MM ..\libgid\src\win32\*.cpp > libgidwin32.dep

g++ %include_paths% -MM ../external/minizip-1.1/source/ioapi.c > ioapi.dep
g++ %include_paths% -MM ../external/minizip-1.1/source/unzip.c > unzip.dep


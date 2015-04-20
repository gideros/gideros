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
-I..\libgid\external\openal-soft-1.16.0\include\AL ^
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

g++ %include_paths% -O2 -c win.cpp 

g++ %include_paths% -U_WIN32 -O2 -c ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Common\*.cpp
g++ %include_paths% -O2 -c ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Collision\*.cpp
g++ %include_paths% -O2 -c ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Collision\Shapes\*.cpp
g++ %include_paths% -O2 -c ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Dynamics\*.cpp
g++ %include_paths% -O2 -c ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Dynamics\Contacts\*.cpp
g++ %include_paths% -O2 -c ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Dynamics\Joints\*.cpp
g++ %include_paths% -O2 -c ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Rope\*.cpp
g++ %include_paths% -O2 -c ..\external\liquidfun-1.0.0\liquidfun\Box2D\Box2D\Particle\*.cpp

g++ %include_paths% -O2 -c ..\2dsg\*.cpp

rem g++ %include_paths% -O2 -c ..\luabinding\*.cpp
rem g++ %include_paths% -O2 -c ..\luabinding\tlsf.c

rem g++ %include_paths% -O2 -c ..\libnetwork\*.cpp

rem g++ %include_paths% -O2 -c ..\libpvrt\*.cpp
rem g++ %include_paths% -O2 -c ..\external\glu\libtess\*.c

rem g++ %include_paths% -O2 -c ..\libgid\src\md5.c
rem g++ %include_paths% -O2 -c ..\libgid\src\utf8.c

rem g++ %include_paths% -O2 -c ..\libgid\src\*.cpp
rem g++ %include_paths% -O2 -c ..\libgid\src\win32\*.cpp

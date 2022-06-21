WebXR library for [Emscripten](https://github.com/emscripten-core/emscripten)

Goal of this project is to expose the [WebXR Device API](https://www.w3.org/TR/webxr/)
for use with C++ through emscripten.

Getting Started
===============

Clone this repository as a submodule to your emscripten-based C++ project.

~~~sh
git submodule add https://github.com/vhiterabbit/emscripten-webxr
~~~

CMake
-----

If you use CMake, you can use `emscripten-webxr` as CMake subproject and link to the `webxr` target.

To link your own executable against `emscripten-webxr`, add this project as a subproject
and see the following example for linking the `webxr` target:

~~~cmake
add_executable(your-own-executable main.cpp)

add_subdirectory(emscripten-webxr)
target_link_libraries(your-own-executable webxr)
~~~

No CMake
--------

Link to `library_webxr.js` to resolve symbols defined in the `webxr.h` header
with the following argument to your emscripten linking step:

~~~sh
--js-library emscripten-webxr/library_webxr.js
~~~

Make sure `webxr.h` is on your include path during compilation.

Usage Example
-------------

For example usage see the [Magnum WebXR example](https://github.com/mosra/magnum-examples/blob/master/src/webxr/WebXrExample.cpp).

Projects using Escmripten WebXR
===============================

 - [Magnum WebXR example](https://github.com/mosra/magnum-examples/blob/master/src/webxr/WebXrExample.cpp) - WebXR example in the Magnum open source C++11/14 OpenGL graphics framework.
 - [Wonderland Engine](https://www.wonderlandengine.com) - Accessible development platform for building highly performant WebXR applications.

LICENSE
=======

[emscripten-webxr](https://github.com/vhiterabbit/emscripten-webxr) is licensed
under the MIT/Expat license, see the [COPYING](COPYING) file
for details.

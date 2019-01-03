#ifndef GIDEROSEXPORT_H
#define GIDEROSEXPORT_H

#ifdef _WIN32
#ifdef GIDEROS_LIBRARY
#define GIDEROS_API __declspec(dllexport)
#else
#ifdef WINSTORE
#define GIDEROS_API
#else
#define GIDEROS_API __declspec(dllimport)
#endif
#endif
#elif __EMSCRIPTEN__
#ifdef GIDEROS_LIBRARY
#include <emscripten.h>
#define GIDEROS_API EMSCRIPTEN_KEEPALIVE
#else
#define GIDEROS_API
#endif
#else
#define GIDEROS_API
#endif

#define G_API GIDEROS_API

#if __EMSCRIPTEN__
#ifdef GIDEROS_LIBRARY
#define G_KEEP EMSCRIPTEN_KEEPALIVE
#else
#define G_KEEP
#endif
#else
#define G_KEEP
#endif

#endif

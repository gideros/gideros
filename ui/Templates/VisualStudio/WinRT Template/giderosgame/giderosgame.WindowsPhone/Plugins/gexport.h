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
#else
#define GIDEROS_API
#endif

#define G_API GIDEROS_API

#endif

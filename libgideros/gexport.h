#ifndef GIDEROSEXPORT_H
#define GIDEROSEXPORT_H

#ifdef _WIN32
#ifdef GIDEROS_LIBRARY
#define GIDEROS_API __declspec(dllexport)
#else
#define GIDEROS_API __declspec(dllimport)
#endif
#else
#define GIDEROS_API
#endif

#endif

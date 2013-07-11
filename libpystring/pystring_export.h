#ifndef PYSTRING_EXPORT_H
#define PYSTRING_EXPORT_H

#ifdef _WIN32
#ifdef PYSTRING_LIBRARY
#define PYSTRING_API __declspec(dllexport)
#else
#define PYSTRING_API __declspec(dllimport)
#endif
#else
#define PYSTRING_API
#endif

#endif

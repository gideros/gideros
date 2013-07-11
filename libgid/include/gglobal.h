#ifndef _GGLOBAL_H_
#define _GGLOBAL_H_

#ifdef _WIN32
#ifdef GID_LIBRARY
#define G_API __declspec(dllexport)
#else
#define G_API __declspec(dllimport)
#endif
#else
#define G_API
#endif

typedef unsigned long g_id;

typedef int g_bool;
#define g_false (0)
#define g_true (1)

G_API g_id g_NextId();

#endif // _GGLOBAL_H_

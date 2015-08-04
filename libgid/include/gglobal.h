#ifndef _GGLOBAL_H_
#define _GGLOBAL_H_

#include <gexport.h>

typedef unsigned long g_id;

typedef int g_bool;
#define g_false (0)
#define g_true (1)

G_API g_id g_NextId();

#endif // _GGLOBAL_H_

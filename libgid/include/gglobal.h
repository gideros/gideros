#ifndef _GGLOBAL_H_
#define _GGLOBAL_H_

#include <gexport.h>

typedef unsigned long g_id;

typedef int g_bool;
#define g_false (0)
#define g_true (1)

#ifdef __cplusplus
extern "C" {
#endif

G_API g_id g_NextId();
G_API double g_iclock();

#ifdef __cplusplus
}
#endif

#endif // _GGLOBAL_H_

#include <gglobal.h>

static g_id s_nextgid = 1;

#ifdef __cplusplus
extern "C" {
#endif

g_id g_NextId()
{
    return s_nextgid++;
}

#ifdef __cplusplus
}
#endif

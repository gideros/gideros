#include <gglobal.h>

static g_id s_nextgid = 1;

g_id g_NextId()
{
    return s_nextgid++;
}

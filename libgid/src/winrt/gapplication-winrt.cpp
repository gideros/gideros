#include <gapplication.h>


extern "C" {

void gapplication_init()
{

}

void gapplication_cleanup()
{

}

int gapplication_getScreenDensity()
{
	return -1;
}

g_id gapplication_addCallback(gevent_Callback callback, void *udata)
{
    return 0;
}

void gapplication_removeCallback(gevent_Callback callback, void *udata)
{

}

void gapplication_removeCallbackWithGid(g_id gid)
{

}

}

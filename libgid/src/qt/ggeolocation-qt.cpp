#include "ggeolocation.h"

#ifdef __cplusplus
extern "C" {
#endif

void ggeolocation_init()
{

}

void ggeolocation_cleanup()
{

}

int ggeolocation_isAvailable()
{
    return 0;
}

int ggeolocation_isHeadingAvailable()
{
    return 0;
}

void ggeolocation_setAccuracy(double accuracy)
{

}

double ggeolocation_getAccuracy()
{
    return 0;
}

void ggeolocation_setThreshold(double threshold)
{

}

double ggeolocation_getThreshold()
{
    return 0;
}

void ggeolocation_startUpdatingLocation()
{

}

void ggeolocation_stopUpdatingLocation()
{

}

void ggeolocation_startUpdatingHeading()
{

}

void ggeolocation_stopUpdatingHeading()
{

}

g_id ggeolocation_addCallback(gevent_Callback callback, void *udata)
{
    return 0;
}

void ggeolocation_removeCallback(gevent_Callback callback, void *udata)
{

}

void ggeolocation_removeCallbackWithGid(g_id gid)
{

}

#ifdef __cplusplus
}
#endif

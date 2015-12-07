#ifndef GAPPLICATION_PI_H
#define GAPPLICATION_PI_H

#include <gglobal.h>
#include <gevent.h>

#ifdef __cplusplus
extern "C" {
#endif

G_API void gapplication_enqueueEvent(int type, void *event, int free);

#ifdef __cplusplus
}
#endif

#endif

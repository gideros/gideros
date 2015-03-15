#ifndef STOREDETECTOR_H
#define STOREDETECTOR_H

#include <gglobal.h>
#include <gevent.h>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

G_API void sd_init();
G_API void sd_cleanup();

G_API int sd_isConsole();
G_API std::string sd_getStore();

#ifdef __cplusplus
}
#endif

#endif

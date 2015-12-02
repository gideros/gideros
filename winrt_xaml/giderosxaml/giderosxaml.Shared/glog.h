#ifndef _GLOG_H_
#define _GLOG_H_

#include <gglobal.h>

#define GLOG_VERBOSE 0
#define GLOG_DEBUG 1
#define GLOG_INFO 2
#define GLOG_WARNING 3
#define GLOG_ERROR 4
#define GLOG_SUPPRESS 5

#ifdef __cplusplus
extern "C" {
#endif

G_API void glog_v(const char *format, ...);
G_API void glog_d(const char *format, ...);
G_API void glog_i(const char *format, ...);
G_API void glog_w(const char *format, ...);
G_API void glog_e(const char *format, ...);

G_API void glog_setLevel(int level);
G_API int glog_getLevel();

#ifdef __cplusplus
}
#endif

#endif


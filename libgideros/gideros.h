#ifndef GIDEROS_H
#define GIDEROS_H

#ifndef GIDEROS_VERSION_ONLY
#include "gplugin.h"

#ifdef __cplusplus
#include "gproxy.h"
#endif

#include "gfile.h"
#include "gpath.h"
#include "glog.h"
#include "gapplication.h"
#include "gevent.h"

#if defined(__has_include)
#if __has_include("gideros_build.h")
# include "gideros_build.h"
#endif
#endif

#endif

#ifndef GIDEROS_VERSION
#define GIDEROS_VERSION "2017.4.1"
#endif

#endif

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

#endif

#if defined(__has_include)
#if __has_include("gideros_build.h")
# include "gideros_build.h"
#endif
#endif

#ifndef GIDEROS_VERSION
#define GIDEROS_VERSION "2020.2"
#endif

#endif

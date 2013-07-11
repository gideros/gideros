#ifndef GFILE_H
#define GFILE_H

#include <stdio.h>

#include "gexport.h"

#ifdef __cplusplus
extern "C" {
#endif

GIDEROS_API const char* g_pathForFile(const char* filename);

#ifdef __cplusplus
}
#endif

#endif

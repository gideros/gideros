#ifndef ___MPG123_H___
#define ___MPG123_H___

#include <stdlib.h>
#include <sys/types.h>

#if defined(_MSC_VER)
#include <BaseTsd.h>
#include <sys/types.h>
typedef SSIZE_T ssize_t;
#endif

#define MPG123_NO_CONFIGURE
#include "mpg123.h.in"

#endif

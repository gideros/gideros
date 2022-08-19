#ifndef __CONFIG_H_INCLUDED__
#define __CONFIG_H_INCLUDED__

#ifndef __MANGLE_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#endif

#define HAVE_STRERROR
#define HAVE_STRDUP
#if !defined(WINSTORE) && !defined(HAVE_UNISTD_H)
#define HAVE_UNISTD_H
#endif

#if defined(_MSC_VER)
#include <BaseTsd.h>
#include <sys/types.h>
typedef SSIZE_T ssize_t;
#define strcasecmp _strcmpi
#define strncasecmp _strnicmp
#define strdup _strdup
#endif

/* We want some frame index, eh? */
#define FRAME_INDEX 1
#define INDEX_SIZE 1000

/* also gapless playback! */
#define GAPLESS 1

#endif

#pragma once

#ifndef __OSDCOMM_H__
#define __OSDCOMM_H__

#include "stdio2.h"
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
#include <typeinfo>
#endif


/***************************************************************************
    COMPILER-SPECIFIC NASTINESS
***************************************************************************/

/* The Win32 port requires this constant for variable arg routines. */
#ifndef CLIB_DECL
#define CLIB_DECL
#endif


/* In C++ we can do type checking via typeid */
#ifdef __cplusplus
#define TYPES_COMPATIBLE(a,b)	(typeid(a) == typeid(b))
#endif


/* Some optimizations/warnings cleanups for GCC */
#if defined(__GNUC__) && (__GNUC__ >= 3)
#define ATTR_UNUSED				__attribute__((__unused__))
#define ATTR_NORETURN			__attribute__((noreturn))
#define ATTR_PRINTF(x,y)		__attribute__((format(printf, x, y)))
#define ATTR_MALLOC				__attribute__((malloc))
#define ATTR_PURE				__attribute__((pure))
#define ATTR_CONST				__attribute__((const))
#define ATTR_FORCE_INLINE		__attribute__((always_inline))
#define ATTR_NONNULL			__attribute__((nonnull(1)))
#define UNEXPECTED(exp)			__builtin_expect((exp), 0)
#define RESTRICT				__restrict__
#define SETJMP_GNUC_PROTECT()	(void)__builtin_return_address(1)
#ifndef TYPES_COMPATIBLE
#define TYPES_COMPATIBLE(a,b)	__builtin_types_compatible_p(typeof(a), b)
#endif
#else
#define ATTR_UNUSED
#define ATTR_NORETURN
#define ATTR_PRINTF(x,y)
#define ATTR_MALLOC
#define ATTR_PURE
#define ATTR_CONST
#define ATTR_FORCE_INLINE
#define ATTR_NONNULL
#define UNEXPECTED(exp)			(exp)
#define RESTRICT
#define SETJMP_GNUC_PROTECT()	do {} while (0)
#ifndef TYPES_COMPATIBLE
#define TYPES_COMPATIBLE(a,b)	1
#endif
#endif


/* And some MSVC optimizations/warnings */
#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#define DECL_NORETURN			__declspec(noreturn)
#else
#define DECL_NORETURN
#endif


/***************************************************************************
    FUNDAMENTAL CONSTANTS
***************************************************************************/

/* Ensure that TRUE/FALSE are defined */
#ifndef TRUE
#define TRUE    			1
#endif

#ifndef FALSE
#define FALSE  				0
#endif



/***************************************************************************
    FUNDAMENTAL MACROS
***************************************************************************/

/* Standard MIN/MAX macros */
#ifndef MIN
#define MIN(x,y)			((x) < (y) ? (x) : (y))
#endif
#ifndef MAX
#define MAX(x,y)			((x) > (y) ? (x) : (y))
#endif


/* U64 and S64 are used to wrap long integer constants. */
#ifdef __GNUC__
#define U64(val) val##ULL
#define S64(val) val##LL
#else
#define U64(val) val
#define S64(val) val
#endif


/* Highly useful macro for compile-time knowledge of an array size */
#define ARRAY_LENGTH(x)		(sizeof(x) / sizeof(x[0]))



#endif	/* __OSDCOMM_H__ */

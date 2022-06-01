#ifndef CONFIG_H
#define CONFIG_H

#define AL_API	extern
#define ALC_API extern

/* Define any available alignment declaration */
#define ALIGN(x) __attribute__((aligned(x)))
#define restrict __restrict

/* Define to the library version */
#define ALSOFT_VERSION "1.17.2"

#ifdef __ANDROID__
#define HAVE_OPENSL
#endif
#ifdef __APPLE__
#define HAVE_COREAUDIO
#endif

/* Define if we have the stat function */
#define HAVE_STAT

/* Define if we have the lrintf function */
#define HAVE_LRINTF

/* Define if we have the modff function */
#define HAVE_MODFF

/* Define if we have the strtof function */
#define HAVE_STRTOF

/* Define if we have the strnlen function */
#define HAVE_STRNLEN

/* Define if we have the __int64 type */
//#define HAVE___INT64

/* Define to the size of a long int type */
#define SIZEOF_LONG 4

/* Define to the size of a long long int type */
#define SIZEOF_LONG_LONG 8

/* Define if we have C99 variable-length array support */
#define HAVE_C99_VLA

/* Define if we have C99 _Bool support */
#define HAVE_C99_BOOL

/* Define if we have C11 _Static_assert support */
#define HAVE_C11_STATIC_ASSERT

/* Define if we have C11 _Alignas support */
//#define HAVE_C11_ALIGNAS

/* Define if we have C11 _Atomic support */
#define HAVE_C11_ATOMIC

/* Define if we have GCC's destructor attribute */
#define HAVE_GCC_DESTRUCTOR

/* Define if we have GCC's format attribute */
#define HAVE_GCC_FORMAT

/* Define if we have stdint.h */
#define HAVE_STDINT_H

/* Define if we have stdbool.h */
#define HAVE_STDBOOL_H

/* Define if we have stdalign.h */
#define HAVE_STDALIGN_H

/* Define if we have windows.h */
#ifdef WIN32
#define HAVE_WINDOWS_H
#endif
#ifdef __ANDROID__
/* Define if we have dlfcn.h */
#define HAVE_DLFCN_H
#endif

/* Define if we have pthread_np.h */
//#define HAVE_PTHREAD_NP_H

#ifdef __ANDROID__
/* Define if we have alloca.h */
#define HAVE_ALLOCA_H
#endif

#ifndef __APPLE__
/* Define if we have malloc.h */
#define HAVE_MALLOC_H
#endif

/* Define if we have dirent.h */
#define HAVE_DIRENT_H

/* Define if we have strings.h */
#define HAVE_STRINGS_H

/* Define if we have cpuid.h */
//#define HAVE_CPUID_H

/* Define if we have intrin.h */
//#define HAVE_INTRIN_H

/* Define if we have sys/sysconf.h */
//#define HAVE_SYS_SYSCONF_H

/* Define if we have guiddef.h */
//#define HAVE_GUIDDEF_H

/* Define if we have initguid.h */
//#define HAVE_INITGUID_H

/* Define if we have ieeefp.h */
//#define HAVE_IEEEFP_H

/* Define if we have float.h */
#define HAVE_FLOAT_H

/* Define if we have fenv.h */
//#define HAVE_FENV_H

/* Define if we have GCC's __get_cpuid() */
//#define HAVE_GCC_GET_CPUID

/* Define if we have the __cpuid() intrinsic */
//#define HAVE_CPUID_INTRINSIC

/* Define if we have _controlfp() */
//#define HAVE__CONTROLFP

/* Define if we have __control87_2() */
//#define HAVE___CONTROL87_2

/* Define if we have pthread_setschedparam() */
#define HAVE_PTHREAD_SETSCHEDPARAM

/* Define if we have pthread_setname_np() */
//#define HAVE_PTHREAD_SETNAME_NP

/* Define if pthread_setname_np() only accepts one parameter */
//#define PTHREAD_SETNAME_NP_ONE_PARAM

/* Define if we have pthread_set_name_np() */
//#define HAVE_PTHREAD_SET_NAME_NP

/* Define if we have pthread_mutexattr_setkind_np() */
//#define HAVE_PTHREAD_MUTEXATTR_SETKIND_NP

/* Define if we have pthread_mutex_timedlock() */
//#define HAVE_PTHREAD_MUTEX_TIMEDLOCK

#endif

#ifndef CONFIG_H
#define CONFIG_H

/* Define to the library version */
#define ALSOFT_VERSION "1.13"

/* Define if we have the ALSA backend */
/* #undef HAVE_ALSA */

/* Define if we have the OSS backend */
/* #undef HAVE_OSS */

/* Define if we have the Solaris backend */
/* #undef HAVE_SOLARIS */

/* Define if we have the DSound backend */
#define HAVE_DSOUND

/* Define if we have the Windows Multimedia backend */
#define HAVE_WINMM

/* Define if we have the PortAudio backend */
/* #undef HAVE_PORTAUDIO */

/* Define if we have the PulseAudio backend */
/* #undef HAVE_PULSEAUDIO */

/* Define if we have the Wave Writer backend */
#define HAVE_WAVE

/* Define if we have dlfcn.h */
/* #undef HAVE_DLFCN_H */

/* Define if we have the stat function */
#define HAVE_STAT

/* Define if we have the powf function */
/* #undef HAVE_POWF */

/* Define if we have the sqrtf function */
/* #undef HAVE_SQRTF */

/* Define if we have the acosf function */
/* #undef HAVE_ACOSF */

/* Define if we have the atanf function */
/* #undef HAVE_ATANF */

/* Define if we have the fabsf function */
/* #undef HAVE_FABSF */

/* Define if we have the strtof function */
/* #undef HAVE_STRTOF */

/* Define if we have stdint.h */
#define HAVE_STDINT_H

/* Define if we have the __int64 type */
/* #undef HAVE___INT64 */

/* Define to the size of a long int type */
#define SIZEOF_LONG 4

/* Define to the size of a long long int type */
#define SIZEOF_LONG_LONG 8

/* Define to the size of an unsigned int type */
#define SIZEOF_UINT 4

/* Define to the size of a void pointer type */
#define SIZEOF_VOIDP 4

/* Define if we have GCC's destructor attribute */
/* #undef HAVE_GCC_DESTRUCTOR */

/* Define if we have GCC's format attribute */
/* #undef HAVE_GCC_FORMAT */

/* Define if we have pthread_np.h */
/* #undef HAVE_PTHREAD_NP_H */

/* Define if we have float.h */
#define HAVE_FLOAT_H

/* Define if we have fenv.h */
/* #undef HAVE_FENV_H */

/* Define if we have fesetround() */
/* #undef HAVE_FESETROUND */

/* Define if we have _controlfp() */
#define HAVE__CONTROLFP

/* Define if we have pthread_setschedparam() */
/* #undef HAVE_PTHREAD_SETSCHEDPARAM */

#endif

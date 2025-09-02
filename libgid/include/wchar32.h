#ifndef _WCHAR32_T_H_
#define _WCHAR32_T_H_

#if defined(__APPLE__)
typedef uint32_t wchar32_t;
#else
#include <uchar.h>
typedef char32_t wchar32_t;
#endif

#endif

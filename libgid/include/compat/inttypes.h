#ifndef __INTTYPES_H__
#define __INTTYPES_H__

#include <BaseTsd.h>
#include <stdint.h>

#define strtoimax _strtoi64
#define strtoumax _strtoui64

#define __signed signed
typedef SSIZE_T ssize_t;

#endif
#ifndef KVEC_H
#define KVEC_H

#include <stdlib.h>
#include <string.h>

#define kvec_t(type) struct { size_t n, m; type *a; }
#define kv_init(v) ((v).n = 0, (v).m = 0, (v).a = NULL)
#define kv_free(v) free((v).a)
#define kv_a(v, i) ((v).a[(i)])
#define kv_size(v) ((v).n)
#define kv_clear(v) ((v).n = 0)
#define kv_data(v) ((v).a)
#define kv_back(v) ((v).a[(v).n - 1])
#define kv_pop_back(v) ((v).n--)
#define kv_empty(v) ((v).n == 0)
#define kv_erase(v, i)															\
	do																			\
	{																			\
		size_t i2 = (i);														\
		memcpy((v).a + i2, (v).a + i2 + 1, ((v).n - i2 - 1) * sizeof(*(v).a));	\
		(v).n--;																\
	} while (0)

#define kv_insert(v, i, x) \
	do																			\
	{																			\
		size_t i2 = (i);														\
		if ((v).n == (v).m)														\
		{																		\
			(v).m = (v).m * 2 + 8;												\
			*(void**)&(v).a = realloc((v).a, sizeof(*(v).a) * (v).m);			\
		}																		\
		memcpy((v).a + i2 + 1, (v).a + i2, ((v).n - i2) * sizeof(*(v).a));		\
		(v).n++;																\
		(v).a[i2] = (x);														\
	} while (0)

#define kv_resize(v, s)													\
	do																	\
	{																	\
		size_t s2 = (s);												\
		if (s2 > (v).m)													\
		{																\
			(v).m = s2;													\
			*(void**)&(v).a = realloc((v).a, sizeof(*(v).a) * (v).m);	\
		}																\
		(v).n = s2;														\
	} while(0)

#define kv_push_back(v, x)												\
	do																	\
	{																	\
		if ((v).n == (v).m)												\
		{																\
			(v).m = (v).m * 2 + 8;										\
			*(void**)&(v).a = realloc((v).a, sizeof(*(v).a) * (v).m);	\
		}																\
		(v).a[(v).n++] = (x);											\
	} while(0)

#endif

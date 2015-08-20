#ifdef __BIG_ENDIAN__
#define BIGENDIAN
#else /* __BIG_ENDIAN__ */
#ifdef __LITTLE_ENDIAN__
#else
#ifdef BSD
#include <sys/endian.h>
#else
#include <endian.h>
#endif
#if __BYTE_ORDER == __BIG_ENDIAN
#define BIGENDIAN
#endif
#endif /* __LITTLE_ENDIAN__ */
#endif /* __BIG_ENDIAN__ */

#include <stdint.h>

#define PACKED __attribute__((packed))
#define WEAK __attribute__((weak))

#define PULONG(p)  ((union unal *)(p))->l
#define PUSHORT(p) ((union unal *)(p))->s
#define PLONG(p)   ((union unal *)(p))->li
#define PSHORT(p)  ((union unal *)(p))->si
#define PUCHAR(p)  ((union unal *)(p))->b
#define PCHAR(p)   ((union unal *)(p))->bi
#define PULONGLONG(p) ((union unal *)(p))->ll
#define PLONGLONG(p) ((union unal *)(p))->lli
#define PFLOAT(p) ((union unal *)(p))->f
#define PDOUBLE(p) ((union unal *)(p))->d

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// Endianness
#define endian_swap2(a) ((((a)&0xFF)<<8)|(((a)>>8)&0xFF))
#define endian_swap4(a) (((endian_swap2(a))<<16)|(endian_swap2((a)>>16)))
#define endian_swap8(a) ((((uquad)(endian_swap4(a)))<<32)|(endian_swap4((a)>>32)))

#ifndef BIGENDIAN
#ifndef LITTLEENDIAN
#define LITTLEENDIAN
#endif
#define BIGENDIAN2(a)		endian_swap2(a)
#define LITTLEENDIAN2(a)	(a)
#define BIGENDIAN4(a)		endian_swap4(a)
#define LITTLEENDIAN4(a)	(a)
#define BIGENDIAN8(a)		endian_swap8(a)
#define LITTLEENDIAN8(a)	(a)
#else
#define BIGENDIAN2(a)		(a)
#define LITTLEENDIAN2(a)	endian_swap2(a)
#define BIGENDIAN4(a)		(a)
#define LITTLEENDIAN4(a)	endian_swap4(a)
#define BIGENDIAN8(a)		(a)
#define LITTLEENDIAN8(a)	endian_swap8(a)
#endif

#define PLULONG(p)  LITTLEENDIAN4(PULONG(p))
#define PLUSHORT(p) LITTLEENDIAN2(PUSHORT(p))
#define PLLONG(p)   ((long)(LITTLEENDIAN4(PULONG(p))))
#define PLSHORT(p)  ((short)(LITTLEENDIAN2(PSHORT(p))))
#define PLULONGLONG(p) LITTLEENDIAN8(PULONGLONG(p))
#define PLLONGLONG(p) ((long long)(LITTLEENDIAN8(PLONGLONG(p))))

#define PBULONG(p)  BIGENDIAN4(PULONG(p))
#define PBUSHORT(p) BIGENDIAN2(PUSHORT(p))
#define PBLONG(p)   ((long)(BIGENDIAN4(PLONG(p))))
#define PBSHORT(p)  ((short)(BIGENDIAN2(PSHORT(p))))
#define PBULONGLONG(p) BIGENDIAN8(PULONGLONG(p))
#define PBLONGLONG(p) ((long long)(BIGENDIAN8(PLONGLONG(p))))

union unal
{
 uint8_t b;
 uint16_t s;
 uint32_t l;
 int8_t bi;
 int16_t si;
 int32_t li;
 uint64_t ll;
 int64_t lli;
 float f;
 double d;
} PACKED;


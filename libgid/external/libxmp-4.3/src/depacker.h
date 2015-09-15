#ifndef XMP_DEPACKER_H
#define XMP_DEPACKER_H

#include "stdio2.h"


extern struct depacker zip_depacker;
extern struct depacker lha_depacker;
extern struct depacker gzip_depacker;
extern struct depacker bzip2_depacker;
extern struct depacker xz_depacker;
extern struct depacker compress_depacker;
extern struct depacker pp_depacker;
extern struct depacker sqsh_depacker;
extern struct depacker arc_depacker;
extern struct depacker arcfs_depacker;
extern struct depacker mmcmp_depacker;
extern struct depacker muse_depacker;
extern struct depacker lzx_depacker;
extern struct depacker s404_depacker;
extern struct depacker xfd_depacker;
extern struct depacker oxm_depacker;

struct depacker {
	int (*const test)(unsigned char *);
	int (*const depack)(FILE *, FILE *);
};

#endif

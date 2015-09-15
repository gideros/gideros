/* ArcFS depacker for xmp
 * Copyright (C) 2007 Claudio Matsuoka
 *
 * Based on the nomarch .arc depacker from nomarch
 * Copyright (C) 2001-2006 Russell Marks
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

#include "stdio2.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "common.h"
#include "depacker.h"
#include "readrle.h"
#include "readhuff.h"
#include "readlzw.h"


struct archived_file_header_tag {
	unsigned char method;
	unsigned char bits;
	char name[13];
	unsigned long compressed_size;
	unsigned int date, time, crc;
	unsigned long orig_size;
	unsigned long offset;
};


static int read_file_header(FILE *in, struct archived_file_header_tag *hdrp)
{
	int hlen, start /*, ver*/;
	int i;

	fseek(in, 8, SEEK_CUR);			/* skip magic */
	hlen = read32l(in) / 36;
	start = read32l(in);
	/*ver =*/ read32l(in);

	read32l(in);
	/*ver =*/ read32l(in);

	fseek(in, 68, SEEK_CUR);		/* reserved */

	for (i = 0; i < hlen; i++) {
		int x = read8(in);

		if (x == 0)			/* end? */
			break;

		hdrp->method = x & 0x7f;
		fread(hdrp->name, 1, 11, in);
		hdrp->name[12] = 0;
		hdrp->orig_size = read32l(in);
		read32l(in);
		read32l(in);
		x = read32l(in);
		hdrp->compressed_size = read32l(in);
		hdrp->offset = read32l(in);

		if (x == 1)			/* deleted */
			continue;

		if (hdrp->offset & 0x80000000)		/* directory */
			continue;
		
		hdrp->crc = x >> 16;
		hdrp->bits = (x & 0xff00) >> 8;
		hdrp->offset &= 0x7fffffff;	
		hdrp->offset += start;	

		break;
	}

	return 1;
}

/* read file data, assuming header has just been read from in
 * and hdrp's data matches it. Caller is responsible for freeing
 * the memory allocated.
 * Returns NULL for file I/O error only; OOM is fatal (doesn't return).
 */
static unsigned char *read_file_data(FILE *in,
				     struct archived_file_header_tag *hdrp)
{
	unsigned char *data;
	int siz = hdrp->compressed_size;

	if ((data = malloc(siz)) == NULL)
		return NULL;

	fseek(in, hdrp->offset, SEEK_SET);
	if (fread(data, 1, siz, in) != siz) {
		free(data);
		data = NULL;
	}

	return data;
}

static int arcfs_extract(FILE *in, FILE *out)
{
	struct archived_file_header_tag hdr;
	unsigned char *data, *orig_data;
	int exitval = 0;

	if (!read_file_header(in, &hdr))
		return -1;

	if (hdr.method == 0)
		return -1;

	/* error reading data (hit EOF) */
	if ((data = read_file_data(in, &hdr)) == NULL)
		return -1;

	orig_data = NULL;

	/* FWIW, most common types are (by far) 8/9 and 2.
	 * (127 is the most common in Spark archives, but only those.)
	 * 3 and 4 crop up occasionally. 5 and 6 are very, very rare.
	 * And I don't think I've seen a *single* file with 1 or 7 yet.
	 */
	switch (hdr.method) {
	case 2:		/* no compression */
		orig_data = data;
		break;

	case 8:		/* "Crunched" [sic]
			 * (RLE+9-to-12-bit dynamic LZW, a *bit* like GIF) */
		orig_data = convert_lzw_dynamic(data, hdr.bits, 1,
					hdr.compressed_size, hdr.orig_size, 0);
		break;

	case 9:		/* "Squashed" (9-to-13-bit, no RLE) */
		orig_data = convert_lzw_dynamic(data, hdr.bits, 0,
					hdr.compressed_size, hdr.orig_size, 0);
		break;

	case 127:	/* "Compress" (9-to-16-bit, no RLE) ("Spark" only) */
		orig_data = convert_lzw_dynamic(data, hdr.bits, 0,
					hdr.compressed_size, hdr.orig_size, 0);
		break;

	default:
		free(data);
		return -1;
	}

	if (orig_data == NULL) {
		free(data);
		return -1;
	}

	if (fwrite(orig_data, 1, hdr.orig_size, out) != hdr.orig_size)
		exitval = -1;

	if (orig_data != data)	/* don't free uncompressed stuff twice :-) */
		free(orig_data);

	free(data);

	return exitval;
}

static int test_arcfs(unsigned char *b)
{
	return !memcmp(b, "Archive\0", 8);
}

static int decrunch_arcfs(FILE * f, FILE * fo)
{
	int ret;

	if (fo == NULL)
		return -1;

	ret = arcfs_extract(f, fo);
	if (ret < 0)
		return -1;

	return 0;
}

struct depacker arcfs_depacker = {
	test_arcfs,
	decrunch_arcfs
};

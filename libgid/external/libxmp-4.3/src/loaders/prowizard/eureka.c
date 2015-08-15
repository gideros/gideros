/*
 * EurekaPacker.c   Copyright (C) 1997 Asle / ReDoX
 *
 * Converts MODs packed with Eureka packer back to ptk
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_eu(HIO_HANDLE *in, FILE *out)
{
	uint8 tmp[1080];
	uint8 c1;
	int npat, smp_addr;
	int ssize = 0;
	int trk_addr[128][4];
	int i, j, k;

	/* read header ... same as ptk */
	hio_read(tmp, 1080, 1, in);
	fwrite(tmp, 1080, 1, out);

	/* now, let's sort out that a bit :) */
	/* first, the whole sample size */
	for (i = 0; i < 31; i++)
		ssize += 2 * readmem16b(tmp + i * 30 + 42);

	/* now, the pattern list .. and the max */
	for (npat = i = 0; i < 128; i++) {
		if (tmp[952 + i] > npat)
			npat = tmp[952 + i];
	}
	npat++;

	write32b(out, PW_MOD_MAGIC);		/* write ptk ID */
	smp_addr = hio_read32b(in);			/* read sample data address */

	/* read tracks addresses */
	for (i = 0; i < npat; i++) {
		for (j = 0; j < 4; j++)
			trk_addr[i][j] = hio_read16b(in);
	}

	/* the track data now ... */
	for (i = 0; i < npat; i++) {
		memset(tmp, 0, 1024);
		for (j = 0; j < 4; j++) {
			hio_seek(in, trk_addr[i][j], SEEK_SET);
			for (k = 0; k < 64; k++) {
				uint8 *x = &tmp[k * 16 + j * 4];
				c1 = hio_read8(in);
				if ((c1 & 0xc0) == 0x00) {
					*x++ = c1;
					*x++ = hio_read8(in);
					*x++ = hio_read8(in);
					*x++ = hio_read8(in);
					continue;
				}
				if ((c1 & 0xc0) == 0xc0) {
					k += (c1 & 0x3f);
					continue;
				}
				if ((c1 & 0xc0) == 0x40) {
					x += 2;
					*x++ = c1 & 0x0f;
					*x++ = hio_read8(in);
					continue;
				}
				if ((c1 & 0xc0) == 0x80) {
					*x++ = hio_read8(in);
					*x++ = hio_read8(in);
					*x++ = (c1 << 4) & 0xf0;
					continue;
				}
			}
		}
		fwrite(tmp, 1024, 1, out);
	}

	hio_seek(in, smp_addr, SEEK_SET);
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_eu(uint8 *data, char *t, int s)
{
	int i;
	int len, max_pat, smp_offs;
	int max_trk, min_trk;

	PW_REQUEST_DATA(s, 1084);

	/* test 2 */
	len = data[950];
	if (len == 0 || len > 127)
		return -1;

	/* test #3  finetunes & volumes */
	for (i = 0; i < 31; i++) {
		uint8 *d = data + i * 30;
		int size = readmem16b(d + 42) << 1;
		int start = readmem16b(d + 46) << 1;
		int lsize = readmem16b(d + 48) << 1;

		if (size > 0xffff || start > 0xffff || lsize > 0xffff)
			return -1;

		if (start + lsize > size + 2)
			return -1;

		if (d[44] > 0x0f || d[45] > 0x40)
			return -1;
	}


	/* test 4 */
	smp_offs = readmem32b(data + 1080);

#if 0
	if (smp_offs > in_size)
		return -1;
#endif

	if (smp_offs < 1084)
		return -1;

	/* pattern list */
	max_pat = 0;
	for (i = 0; i < len; i++) {
		int pat = data[952 + i];
		if (pat > 127)
			return -1;
		if (pat > max_pat)
			max_pat = pat;
	}
	for (/*i += 2*/; i < 128; i++) {
		if (data[952 + i] != 0)
			return -1;
	}

	max_pat++;

	/* test #5 */
	/* max_trkptr is the highest track address */
	/* min_trkptr is the lowest track address */
	max_trk = 0;
	min_trk = 999999;

	PW_REQUEST_DATA(s, max_pat * 4 * 2 + 1085);

	for (i = 0; i < (max_pat * 4); i++) {
		int trk = readmem16b(data + i * 2 + 1084);
		if (trk > smp_offs || trk < 1084)
			return -1;
		if (trk > max_trk)
			max_trk = trk;
		if (trk < min_trk)
			min_trk = trk;
	}

	/* test track datas */
	/* last track wont be tested ... */
	for (i = min_trk; i < max_trk; i++) {
		if ((data[i] & 0xc0) == 0xc0)
			continue;

		if ((data[i] & 0xc0) == 0x80) {
			i += 2;
			continue;
		}

		if ((data[i] & 0xc0) == 0x40) {
			if ((data[i] & 0x3f) == 0 && data[i + 1] == 0)
				return -1;
			i++;
			continue;
		}

		if ((data[i] & 0xc0) == 0) {
			if (data[i] > 0x13)
				return -1;
			i += 3;
			continue;
		}
	}

	pw_read_title(data, t, 20);

	return 0;
}

const struct pw_format pw_eu = {
	"Eureka Packer",
	test_eu,
	depack_eu
};

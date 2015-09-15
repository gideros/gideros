/*
 * Unic_Tracker_2.c   Copyright (C) 1997 Asle / ReDoX
 *
 * Convert Unic Tracker 2 MODs to Protracker
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_unic2(HIO_HANDLE *in, FILE *out)
{
	uint8 c1, c2, c3, c4;
	uint8 npat, maxpat;
	uint8 ins, note, fxt, fxp;
	uint8 fine;
	uint8 tmp[1025];
	int i, j;
	int ssize;

	pw_write_zero(out, 20);		/* title */

	ssize = 0;
	for (i = 0; i < 31; i++) {
		int len, start, lsize;

		pw_move_data(out, in, 20);	/* sample name */
		write8(out, 0);
		write8(out, 0);

		/* fine on ? */
		c1 = hio_read8(in);
		c2 = hio_read8(in);
		j = (c1 << 8) + c2;

		if (j != 0) {
			if (j < 256)
				fine = 0x10 - c2;
			else
				fine = 0x100 - c2;
		} else {
			fine = 0;
		}

		/* smp size */
		len = hio_read16b(in);
		write16b(out, len);
		ssize += len << 1;

		hio_read8(in);
		write8(out, fine);		/* fine */
		write8(out, hio_read8(in));		/* vol */

		start = hio_read16b(in);		/* loop start */
		lsize = hio_read16b(in);		/* loop size */

		if (start * 2 + lsize <= len && start != 0) {
			start <<= 1;
		}

		write16b(out, start);
		write16b(out, lsize);
	}

	write8(out, npat = hio_read8(in));		/* number of pattern */
	write8(out, 0x7f);			/* noisetracker byte */
	hio_read8(in);

	hio_read(tmp, 128, 1, in);
	fwrite(tmp, 128, 1, out);		/* pat table */

	/* get highest pattern number */
	for (maxpat = i = 0; i < 128; i++) {
		if (tmp[i] > maxpat)
			maxpat = tmp[i];
	}
	maxpat++;		/* coz first is $00 */

	write32b(out, PW_MOD_MAGIC);

	/* pattern data */
	for (i = 0; i < maxpat; i++) {
		for (j = 0; j < 256; j++) {
			c1 = hio_read8(in);
			c2 = hio_read8(in);
			c3 = hio_read8(in);

			ins = ((c1 >> 2) & 0x10) | ((c2 >> 4) & 0x0f);
			note = c1 & 0x3f;
			fxt = c2 & 0x0f;
			fxp = c3;

			if (fxt == 0x0d) {	/* pattern break */
				c4 = fxp % 10;
				c3 = fxp / 10;
				fxp = 16 * c3 + c4;
			}

			tmp[j * 4] = (ins & 0xf0);
			tmp[j * 4] |= ptk_table[note][0];
			tmp[j * 4 + 1] = ptk_table[note][1];
			tmp[j * 4 + 2] = ((ins << 4) & 0xf0) | fxt;
			tmp[j * 4 + 3] = fxp;
		}
		fwrite(tmp, 1024, 1, out);
	}

	/* sample data */
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_unic2(uint8 *data, char *t, int s)
{
	int i;
	int len, psize, ssize, max_ins;

	/* test 1 */
	PW_REQUEST_DATA (s, 1084);

	/* test #2 ID = $00000000 ? */
	if (readmem32b(data + 1080) == 0x00000000)
		return -1;

	/* test 2,5 :) */
	ssize = 0;
	max_ins = 0;
	for (i = 0; i < 31; i++) {
		uint8 *d = data + i * 30;

		int size = readmem16b(d + 22) << 1;
		int start = readmem16b(d + 26) << 1;
		int lsize = readmem16b(d + 28) << 1;
		ssize += size;

		if (size + 2 < start + lsize)
			return -1;

		if (size > 0xffff || start > 0xffff || lsize > 0xffff)
			return -1;

		if (d[25] > 0x40)
			return -1;

		if (readmem16b(d + 20) && size == 0)
			return -1;

		if (d[25] != 0 && size == 0)
			return -1;

		/* get the highest !0 sample */
		if (size != 0)
			max_ins = i + 1;
	}

	if (ssize <= 2)
		return -1;

	/* test #4  pattern list size */
	len = data[930];
	if (len == 0 || len > 127)
		return -1;

	psize = 0;
	for (i = 0; i < len; i++) {
		int x = data[932 + i];
		if (x > 127)
			return -1;
		if (x > psize)
			psize = x;
	}

	/* test last patterns of the pattern list = 0 ? */
	for (i += 2; i != 128; i++) {
		if (data[932 + i] != 0)
			return -1;
	}

	psize++;
	psize <<= 8;

	PW_REQUEST_DATA (s, 1060 + psize * 3 + 2);

	for (i = 0; i < psize; i++) {
		uint8 *d = data + 1060 + i * 3;
		int ins;

		/* relative note number + last bit of sample > $34 ? */
		if (d[0] > 0x74)
			return -1;
		if ((d[0] & 0x3F) > 0x24)
			return -1;
		if ((d[1] & 0x0F) == 0x0C && d[2] > 0x40)
			return -1;
		if ((d[1] & 0x0F) == 0x0B && d[2] > 0x7F)
			return -1;
		if ((d[1] & 0x0F) == 0x0D && d[2] > 0x40)
			return -1;

		ins = ((d[0] >> 2) & 0x30) | ((d[2] >> 4) & 0x0f);

		if (ins > max_ins)
			return -1;
	}

	pw_read_title(NULL, t, 0);

	return 0;
}

const struct pw_format pw_unic2 = {
	"Unic Tracker 2",
	test_unic2,
	depack_unic2
};

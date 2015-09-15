/*
 * gmc.c    Copyright (C) 1997 Sylvain "Asle" Chipaux
 *
 * Depacks musics in the Game Music Creator format and saves in ptk.
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_GMC(HIO_HANDLE *in, FILE *out)
{
	uint8 tmp[1024];
	uint8 ptable[128];
	uint8 max;
	uint8 pat_pos;
	uint16 len, looplen;
	long ssize = 0;
	long i = 0, j = 0;

	memset(ptable, 0, 128);

	pw_write_zero(out, 20);			/* title */

	for (i = 0; i < 15; i++) {
		pw_write_zero(out, 22);		/* name */
		hio_read32b(in);			/* bypass 4 address bytes */
		len = hio_read16b(in);
		write16b(out, len);		/* size */
		ssize += len * 2;
		hio_read8(in);
		write8(out, 0);			/* finetune */
		write8(out, hio_read8(in));		/* volume */
		hio_read32b(in);			/* bypass 4 address bytes */

		looplen = hio_read16b(in);		/* loop size */
		write16b(out, looplen > 2 ? len - looplen : 0);
		write16b(out, looplen <= 2 ? 1 : looplen);
		hio_read16b(in);			/* always zero? */
	}

	memset(tmp, 0, 30);
	tmp[29] = 0x01;
	for (i = 0; i < 16; i++)
		fwrite(tmp, 30, 1, out);

	hio_seek(in, 0xf3, 0);
	write8(out, pat_pos = hio_read8(in));	/* pattern list size */
	write8(out, 0x7f);			/* ntk byte */

	/* read and write size of pattern list */
	/*printf ( "Creating the pattern table ... " ); */
	for (i = 0; i < 100; i++)
		ptable[i] = hio_read16b(in) / 1024;
	fwrite(ptable, 128, 1, out);

	/* get number of pattern */
	for (max = i = 0; i < 128; i++) {
		if (ptable[i] > max)
			max = ptable[i];
	}

	/* write ID */
	write32b(out, PW_MOD_MAGIC);

	/* pattern data */
	hio_seek(in, 444, SEEK_SET);
	for (i = 0; i <= max; i++) {
		memset(tmp, 0, 1024);
		hio_read(tmp, 1024, 1, in);
		for (j = 0; j < 256; j++) {
			switch (tmp[(j * 4) + 2] & 0x0f) {
			case 3:	/* replace by C */
				tmp[(j * 4) + 2] += 0x09;
				break;
			case 4:	/* replace by D */
				tmp[(j * 4) + 2] += 0x09;
				break;
			case 5:	/* replace by B */
				tmp[(j * 4) + 2] += 0x06;
				break;
			case 6:	/* replace by E0 */
				tmp[(j * 4) + 2] += 0x08;
				break;
			case 7:	/* replace by E0 */
				tmp[(j * 4) + 2] += 0x07;
				break;
			case 8:	/* replace by F */
				tmp[(j * 4) + 2] += 0x07;
				break;
			default:
				break;
			}
		}
		fwrite(tmp, 1024, 1, out);
	}

	/* sample data */
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_GMC(uint8 *data, char *t, int s)
{
	int i, j, k;
	int ssize, numpat;

	PW_REQUEST_DATA(s, 1024);

#if 0
	/* test #1 */
	if (i < 7) {
		return -1;
	}
	start = i - 7;
#endif

	/* samples descriptions */
	ssize = 0;
	for (i = 0; i < 15; i++) {
		int len, lsize;
		uint8 *d = data + 16 * i;

		/* volumes */
		if (d[7] > 0x40)
			return -1;

		len = readmem16b(d + 4) << 1;
		lsize = readmem16b(d + 12);

		/* size */
		if (len > 0xffff)
			return -1;

		if (lsize > len)
			return -1;

		ssize += len;
	}
	if (ssize <= 4)
		return -1;

	/* pattern table size */
	if (data[243] > 0x64 || data[243] == 0)
		return -1;

	/* pattern order table */
	numpat = 0;
	for (i = 0; i < 100; i++) {
		k = readmem16b(data + 244 + i * 2);
		if (k & 0x03ff)
			return -1;
		if ((k >> 10) > numpat)
			numpat = k >> 10;
	}
	numpat++;

	if (numpat == 1 || numpat > 100)
		return -1;

	PW_REQUEST_DATA(s, 444 + k * 1024 + i * 4 + 3);

	/* test pattern data */
	for (i = 0; i < numpat; i++) {
		for (j = 0; j < 256; j++) {
			int offset = 444 + i * 1024 + j * 4;
			uint8 *d = &data[offset];

			if (offset > (PW_TEST_CHUNK - 4))
				return -1;
				
			/* First test fails with Jumping Jackson */
			if (/*d[0] > 0x03 ||*/ (d[2] & 0x0f) >= 0x90)
				return -1;
#if 0
			/* Test fails with Jumping Jackson */
			/* x is the highest jot jull sample */
			if (((d[2] & 0xf0) >> 4) > x)
				return -1;
#endif
			if ((d[2] & 0x0f) == 3 && d[3] > 0x40)
				return -1;

			if ((d[2] & 0x0f) == 4 && d[3] > 0x63)
				return -1;

			if ((d[2] & 0x0f) == 5 && d[3] > (data[243] + 1))
				return -1;

			if ((d[2] & 0x0f) == 6 && d[3] >= 0x02)
				return -1;

			if ((d[2] & 0x0f) == 7 && d[3] >= 0x02)
				return -1;
		}
	}

	pw_read_title(NULL, t, 0);

	return 0;
}

const struct pw_format pw_gmc = {
	"Game Music Creator",
	test_GMC,
	depack_GMC
};

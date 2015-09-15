/*
 *  NoiseRunner.c	Copyright (C) 1997 Asle / ReDoX
 *
 *  Converts NoiseRunner packed MODs back to Protracker
 *
 *  Modified in 2010,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int fine_table[] = {
	0x0000, 0xffb8, 0xff70, 0xff28, 0xfee0, 0xfe98, 0xfe50, 0xfe08,
	0xfdc0, 0xfd78, 0xfd30, 0xfce8, 0xfca0, 0xfc58, 0xfc10, 0xfbc8
};


static int depack_nru(HIO_HANDLE *in, FILE *out)
{
	uint8 tmp[1025];
	uint8 ptable[128];
	uint8 note, ins, fxt, fxp;
	uint8 pat_data[1025];
	int fine;
	int i, j;
	int max_pat;
	long ssize = 0;

	pw_write_zero(out, 20);			/* title */

	for (i = 0; i < 31; i++) {		/* 31 samples */
		int vol, size, addr, start, lsize;

		pw_write_zero(out, 22);		/* sample name */
		hio_read8(in);			/* bypass 0x00 */
		vol = hio_read8(in);		/* read volume */
		addr = hio_read32b(in);		/* read sample address */
		write16b(out, size = hio_read16b(in)); /* read/write sample size */
		ssize += size * 2;
		start = hio_read32b(in);		/* read loop start address */

		lsize = hio_read16b(in);		/* read loop size */
		fine = hio_read16b(in);		/* read finetune ?!? */

		for (j = 0; j < 16; j++) {
			if (fine == fine_table[j]) {
				fine = j;
				break;
			}
		}
		if (j == 16)
			fine = 0;

		write8(out, fine);		/* write fine */
		write8(out, vol);		/* write vol */
		write16b(out, (start - addr) / 2);	/* write loop start */
		write16b(out, lsize);		/* write loop size */
	}

	hio_seek(in, 950, SEEK_SET);
	write8(out, hio_read8(in));			/* size of pattern list */
	write8(out, hio_read8(in));			/* ntk byte */

	/* pattern table */
	max_pat = 0;
	hio_read(ptable, 128, 1, in);
	fwrite(ptable, 128, 1, out);
	for (i = 0; i < 128; i++) {
		if (ptable[i] > max_pat)
			max_pat = ptable[i];
	}
	max_pat++;

	write32b(out, PW_MOD_MAGIC);

	/* pattern data */
	hio_seek(in, 0x043c, SEEK_SET);
	for (i = 0; i < max_pat; i++) {
		memset(pat_data, 0, 1025);
		hio_read(tmp, 1024, 1, in);
		for (j = 0; j < 256; j++) {
			ins = (tmp[j * 4 + 3] >> 3) & 0x1f;
			note = tmp[j * 4 + 2];
			fxt = tmp[j * 4];
			fxp = tmp[j * 4 + 1];
			switch (fxt) {
			case 0x00:	/* tone portamento */
				fxt = 0x03;
				break;
			case 0x0C:	/* no fxt */
				fxt = 0x00;
				break;
			default:
				fxt >>= 2;
				break;
			}
			pat_data[j * 4] = ins & 0xf0;
			pat_data[j * 4] |= ptk_table[note / 2][0];
			pat_data[j * 4 + 1] = ptk_table[note / 2][1];
			pat_data[j * 4 + 2] = (ins << 4) & 0xf0;
			pat_data[j * 4 + 2] |= fxt;
			pat_data[j * 4 + 3] = fxp;
		}
		fwrite (pat_data, 1024, 1, out);
	}

	pw_move_data(out, in, ssize);		/* sample data */

	return 0;
}

static int test_nru(uint8 *data, char *t, int s)
{
	int i;
	int len, psize, ssize;

	PW_REQUEST_DATA(s, 1500);

#if 0
	/* test 1 */
	if (i < 1080) {
		return -1;
	}
#endif

	if (readmem32b(data + 1080) != PW_MOD_MAGIC)
		return -1;

	/* test 2 */
	ssize = 0;
	for (i = 0; i < 31; i++)
		ssize += 2 * readmem16b(data + 6 + i * 16);
	if (ssize == 0)
		return -1;

	/* test #3 volumes */
	for (i = 0; i < 31; i++) {
		if (data[1 + i * 16] > 0x40)
			return -1;
	}

	/* test #4  pattern list size */
	len = data[950];
	if (len == 0 || len > 127) {
		return -1;
	}

	/* l holds the size of the pattern list */
	psize = 0;
	for (i = 0; i < len; i++) {
		int x = data[952 + i];
		if (x > psize)
			psize = x;
		if (x > 127) {
			return -1;
		}
	}
	/* test last patterns of the pattern list = 0 ? */
	while (i != 128) {
		if (data[952 + i] != 0) {
			return -1;
		}
		i++;
	}

	psize++;
	psize <<= 8;

	/* test #5 pattern data ... */
	for (i = 0; i < psize; i++) {
		uint8 *d = data + 1084 + i * 4;
		/* note > 48h ? */
		if (d[2] > 0x48)
			return -1;
		if (d[3] & 0x07)
			return -1;
		if (d[0] & 0x03)
			return -1;
	}

	pw_read_title(NULL, t, 0);

	return 0;
}

const struct pw_format pw_nru = {
	"NoiseRunner",
	test_nru,
	depack_nru
};

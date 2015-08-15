/*
 * XANN_Packer.c   Copyright (C) 1997 Asle / ReDoX
 *
 * XANN Packer to Protracker.
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include "prowiz.h"

#define SMP_DESC_ADDRESS 0x206
#define PAT_DATA_ADDRESS 0x43C


static int depack_xann(HIO_HANDLE *in, FILE *out)
{
	uint8 c1, c2, c5;
	uint8 ptable[128];
	uint8 pat = 0x00;
	uint8 note, ins, fxt, fxp;
	uint8 fine, vol;
	uint8 pdata[1025];
	int i, j, k;
	int size, ssize = 0;
	int lsize;

	memset(ptable, 0, 128);
	memset(pdata, 0, 1025);

	pw_write_zero(out, 20);			/* title */

	/* 31 samples */
	hio_seek(in, SMP_DESC_ADDRESS, SEEK_SET);

	for (i = 0; i < 31; i++) {
		pw_write_zero(out, 22);		/* sample name */

		fine = hio_read8(in);		/* read finetune */
		vol = hio_read8(in);		/* read volume */
		j = hio_read32b(in);		/* read loop start address */
		lsize = hio_read16b(in);		/* read loop size */
		k = hio_read32b(in);		/* read sample address */
		write16b(out, size = hio_read16b(in)); 	/* sample size */
		ssize += size * 2;

		j = j - k;			/* calculate loop start value */
		write8(out, fine);		/* write fine */
		write8(out, vol);		/* write vol */
		write16b(out, j / 2);		/* write loop start */
		write16b(out, lsize);		/* write loop size */

		hio_read16b(in);			/* bypass two unknown bytes */
	}

	/* pattern table */
	hio_seek(in, 0, SEEK_SET);

	for (pat = c5 = 0; c5 < 128; c5++) {
		k = hio_read32b(in);
		if (k == 0)
			break;
		ptable[c5] = ((k - 0x3c) / 1024) - 1;
		if (ptable[c5] > pat)
			pat = ptable[c5];
	}
	pat++;				/* starts at $00 */

	write8(out, c5);		/* write number of pattern */
	write8(out, 0x7f);		/* write noisetracker byte */

	fwrite(ptable, 128, 1, out);	/* write pattern list */
	write32b(out, PW_MOD_MAGIC);	/* write Protracker's ID */

	/* pattern data */
	hio_seek(in, PAT_DATA_ADDRESS, SEEK_SET);

	for (i = 0; i < pat; i++) {
		for (j = 0; j < 256; j++) {
			uint8 *p = pdata + j * 4;

			ins = (hio_read8(in) >> 3) & 0x1f;
			note = hio_read8(in);
			fxt = hio_read8(in);
			fxp = hio_read8(in);

			switch (fxt) {
			case 0x00:	/* no fxt */
				fxt = 0x00;
				break;
			case 0x04:	/* arpeggio */
				fxt = 0x00;
				break;
			case 0x08:	/* portamento up */
				fxt = 0x01;
				break;
			case 0x0C:	/* portamento down */
				fxt = 0x02;
				break;
			case 0x10:	/* tone portamento with no fxp */
				fxt = 0x03;
				break;
			case 0x14:	/* tone portamento */
				fxt = 0x03;
				break;
			case 0x18:	/* vibrato with no fxp */
				fxt = 0x04;
				break;
			case 0x1C:	/* vibrato */
				fxt = 0x04;
				break;
			case 0x24:	/* tone portamento + vol slide DOWN */
				fxt = 0x05;
				break;
			case 0x28:	/* vibrato + volume slide UP */
				fxt = 0x06;
				c1 = (fxp << 4) & 0xf0;
				c2 = (fxp >> 4) & 0x0f;
				fxp = c1 | c2;
				break;
			case 0x2C:	/* vibrato + volume slide DOWN */
				fxt = 0x06;
				break;
			case 0x38:	/* sample offset */
				fxt = 0x09;
				break;
			case 0x3C:	/* volume slide up */
				fxt = 0x0A;
				c1 = (fxp << 4) & 0xf0;
				c2 = (fxp >> 4) & 0x0f;
				fxp = c1 | c2;
				break;
			case 0x40:	/* volume slide down */
				fxt = 0x0A;
				break;
			case 0x44:	/* position jump */
				fxt = 0x0B;
				break;
			case 0x48:	/* set volume */
				fxt = 0x0C;
				break;
			case 0x4C:	/* pattern break */
				fxt = 0x0D;
				break;
			case 0x50:	/* set speed */
				fxt = 0x0F;
				break;
			case 0x58:	/* set filter */
				fxt = 0x0E;
				fxp = 0x01;
				break;
			case 0x5C:	/* fine slide up */
				fxt = 0x0E;
				fxp |= 0x10;
				break;
			case 0x60:	/* fine slide down */
				fxt = 0x0E;
				fxp |= 0x20;
				break;
			case 0x84:	/* retriger */
				fxt = 0x0E;
				fxp |= 0x90;
				break;
			case 0x88:	/* fine volume slide up */
				fxt = 0x0E;
				fxp |= 0xa0;
				break;
			case 0x8C:	/* fine volume slide down */
				fxt = 0x0E;
				fxp |= 0xb0;
				break;
			case 0x94:	/* note delay */
				fxt = 0x0E;
				fxp |= 0xd0;
				break;
			case 0x98:	/* pattern delay */
				fxt = 0x0E;
				fxp |= 0xe0;
				break;
			default:
				fxt = fxp = 0;
				break;
			}

			p[0] = ins & 0xf0;
			p[0] |= ptk_table[note >> 1][0];
			p[1] = ptk_table[note >> 1][1];
			p[2] = (ins << 4) & 0xf0;
			p[2] |= fxt;
			p[3] = fxp;
		}

		fwrite(pdata, 1024, 1, out);
	}

	/* sample data */
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_xann(uint8 *data, char *t, int s)
{
	int i;

	PW_REQUEST_DATA(s, 2048);

	/* test 1 */
	if (data[3] != 0x3c)
		return -1;

	/* test 2 */
	for (i = 0; i < 128; i++) {
		uint32 j = readmem32b(data + i * 4);
		uint32 k = j & ~3;
		if (k != j || j > 132156)
			return -1;
	}

#if 0
	/* test 3 */
	if (size < 2108)
		return -1;
#endif

	/* test 4 */
	for (i = 0; i < 64; i++) {
		if (data[3 + i * 4] != 0x3c &&
			data[3 + i * 4] != 0) {
			return -1;
		}
	}

	/* test 5 */
	for (i = 0; i < 31; i++) {
		if (data[519 + 16 * i] > 0x40)
			return -1;
	}

	/* test #6  (address of samples) */
	for (i = 0; i < 30; i++) {
		uint32 j = readmem32b(data + 526 + 16 * i);
		/* j = readmem16b(data + 524 + 16 * i) * 2; */
		uint32 k = readmem32b(data + 520 + 16 * (i + 1));

		if (j < 2108 || k < 2108)
			return -1;

		if (j > k)
			return -1;
	}

#if 0
	/* test #7  first pattern data .. */
	for (j = 0; j < 256; j++) {
#if 0
		k = data[j * 4 + 1085] / 2;
		l = k * 2;
		if (data[j * 4 + 1085] != l)
			return -1;
#endif
		if (data[j * 4 + 1085] & 1)
			return -1;
	}
#endif

	pw_read_title(NULL, t, 0);

	return 0;
}

const struct pw_format pw_xann = {
	"XANN Packer",
	test_xann,
	depack_xann
};

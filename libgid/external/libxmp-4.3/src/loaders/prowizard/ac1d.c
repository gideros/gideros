/*
 * ac1d.c   Copyright (C) 1996-1997 Asle / ReDoX
 *
 * Converts ac1d packed MODs back to PTK MODs
 * thanks to Gryzor and his ProWizard tool ! ... without it, this prog
 * would not exist !!!
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <stdlib.h>
#include <string.h>
#include "prowiz.h"

#define NO_NOTE 0xff

static int depack_ac1d(HIO_HANDLE *in, FILE *out)
{
	uint8 c1, c2, c3, c4;
	uint8 npos;
	uint8 ntk_byte;
	uint8 tmp[1024];
	uint8 npat;
	uint8 note, ins, fxt, fxp;
	int size;
	int saddr;
	int ssize = 0;
	int paddr[128];
	int psize[128];
	/*int tsize1, tsize2, tsize3;*/
	int i, j, k;

	memset(paddr, 0, 128 * 4);
	memset(psize, 0, 128 * 4);

	npos = hio_read8(in);
	ntk_byte = hio_read8(in);
	hio_read16b(in);			/* bypass ID */
	saddr = hio_read32b(in);		/* sample data address */

	pw_write_zero(out, 20);		/* write title */

	for (i = 0; i < 31; i++) {
		pw_write_zero(out, 22);		/* name */
		write16b(out, size = hio_read16b(in));	/* size */
		ssize += size * 2;
		write8(out, hio_read8(in));		/* finetune */
		write8(out, hio_read8(in));		/* volume */
		write16b(out, hio_read16b(in));	/* loop start */
		write16b(out, hio_read16b(in));	/* loop size */
	}

	/* pattern addresses */
	for (npat = 0; npat < 128; npat++) {
		paddr[npat] = hio_read32b(in);
		if (paddr[npat] == 0)
			break;
	}
	if (npat == 0) {
		return -1;
	}

	npat--;

	for (i = 0; i < (npat - 1); i++)
		psize[i] = paddr[i + 1] - paddr[i];

	write8(out, npos);		/* write number of pattern pos */
	write8(out, ntk_byte);		/* write "noisetracker" byte */

	hio_seek(in, 0x300, SEEK_SET);	/* go to pattern table .. */
	pw_move_data(out, in, 128);	/* pattern table */
	
	write32b(out, PW_MOD_MAGIC);	/* M.K. */

	/* pattern data */
	for (i = 0; i < npat; i++) {
		hio_seek(in, paddr[i], SEEK_SET);
		/*tsize1 =*/ hio_read32b(in);
		/*tsize2 =*/ hio_read32b(in);
		/*tsize3 =*/ hio_read32b(in);

		memset(tmp, 0, 1024);
		for (k = 0; k < 4; k++) {
			for (j = 0; j < 64; j++) {
				int x = j * 16 + k * 4;

				c1 = hio_read8(in);
				if (c1 & 0x80) {
					c4 = c1 & 0x7f;
					j += (c4 - 1);
					continue;
				}

				c2 = hio_read8(in);
				ins = ((c1 & 0xc0) >> 2) | ((c2 >> 4) & 0x0f);
				note = c1 & 0x3f;

				if (note == 0x3f)
					note = NO_NOTE;
				else if (note)
					note -= 0x0b;

				if (note == 0)
					note++;

				tmp[x] = ins & 0xf0;

				if (note != NO_NOTE) {
					tmp[x] |= ptk_table[note][0];
					tmp[x + 1] = ptk_table[note][1];
				}

				if ((c2 & 0x0f) == 0x07) {
					tmp[x + 2] = (ins << 4) & 0xf0;
					continue;
				}

				c3 = hio_read8(in);
				fxt = c2 & 0x0f;
				fxp = c3;
				tmp[x + 2] = ((ins << 4) & 0xf0) | fxt;
				tmp[x + 3] = fxp;
			}
		}
		fwrite(tmp, 1024, 1, out);
	}

	/* sample data */
	hio_seek(in, saddr, 0);
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_ac1d(uint8 *data, char *t, int s)
{
	int i;

	PW_REQUEST_DATA(s, 896);

	/* test #1 */
	if (data[2] != 0xac || data[3] != 0x1d)
		return -1;

	/* test #2 */
	if (data[0] > 0x7f)
		return -1;

	/* test #4 */
	for (i = 0; i < 31; i++) {
		if (data[10 + 8 * i] > 0x0f)
			return -1;
	}

	/* test #5 */
	for (i = 0; i < 128; i++) {
		if (data[768 + i] > 0x7f)
			return -1;
	}

	pw_read_title(NULL, t, 0);

	return 0;
}

const struct pw_format pw_ac1d = {
	"AC1D Packer",
	test_ac1d,
	depack_ac1d
};


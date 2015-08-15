/*
 * Zen_Packer.c   Copyright (C) 1998 Asle / ReDoX
 *
 * Converts ZEN packed MODs back to PTK MODs
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_zen(HIO_HANDLE *in, FILE *out)
{
	uint8 c1, c2, c3, c4;
	uint8 finetune, vol;
	uint8 pat_pos;
	uint8 pat_max;
	uint8 note, ins, fxt, fxp;
	uint8 pat[1024];
	uint8 ptable[128];
	int size, ssize = 0;
	int paddr[128];
	int paddr_Real[128];
	int ptable_addr;
	int sdata_addr = 999999l;
	int i, j, k;

	memset(paddr, 0, 128 * 4);
	memset(paddr_Real, 0, 128 * 4);
	memset(ptable, 0, 128);

	ptable_addr = hio_read32b(in);	/* read pattern table address */
	pat_max = hio_read8(in);		/* read patmax */
	pat_pos = hio_read8(in);		/* read size of pattern table */

	pw_write_zero(out, 20);		/* write title */

	for (i = 0; i < 31; i++) {
		pw_write_zero(out, 22);			/* sample name */

		finetune = hio_read16b(in) / 0x48;		/* read finetune */

		hio_read8(in);
		vol = hio_read8(in);			/* read volume */

		write16b(out, size = hio_read16b(in));	/* read sample size */
		ssize += size * 2;

		write8(out, finetune);			/* write finetune */
		write8(out, vol);			/* write volume */

		size = hio_read16b(in);			/* read loop size */

		k = hio_read32b(in);			/* sample start addr */
		if (k < sdata_addr)
			sdata_addr = k;

		/* read loop start address */
		j = (hio_read32b(in) - k) / 2;

		write16b(out, j);	/* write loop start */
		write16b(out, size);	/* write loop size */
	}

	write8(out, pat_pos);		/* write size of pattern list */
	write8(out, 0x7f);		/* write ntk byte */

	/* read pattern table .. */
	hio_seek(in, ptable_addr, SEEK_SET);
	for (i = 0; i < pat_pos; i++)
		paddr[i] = hio_read32b(in);

	/* deduce pattern list */
	c4 = 0;
	for (i = 0; i < pat_pos; i++) {
		if (i == 0) {
			ptable[0] = 0;
			paddr_Real[0] = paddr[0];
			c4++;
			continue;
		}
		for (j = 0; j < i; j++) {
			if (paddr[i] == paddr[j]) {
				ptable[i] = ptable[j];
				break;
			}
		}
		if (j == i) {
			paddr_Real[c4] = paddr[i];
			ptable[i] = c4;
			c4++;
		}
	}

	fwrite(ptable, 128, 1, out);		/* write pattern table */
	write32b(out, PW_MOD_MAGIC);		/* write ptk ID */

	/* pattern data */
	/*printf ( "converting pattern datas " ); */
	for (i = 0; i <= pat_max; i++) {
		memset(pat, 0, 1024);
		hio_seek(in, paddr_Real[i], SEEK_SET);
		for (j = 0; j < 256; j++) {
			c1 = hio_read8(in);
			c2 = hio_read8(in);
			c3 = hio_read8(in);
			c4 = hio_read8(in);

			note = (c2 & 0x7f) / 2;
			fxp = c4;
			ins = ((c2 << 4) & 0x10) | ((c3 >> 4) & 0x0f);
			fxt = c3 & 0x0f;

			k = c1;
			pat[k * 4] = ins & 0xf0;
			pat[k * 4] |= ptk_table[note][0];
			pat[k * 4 + 1] = ptk_table[note][1];
			pat[k * 4 + 2] = fxt | ((ins << 4) & 0xf0);
			pat[k * 4 + 3] = fxp;
			j = c1;
		}
		fwrite (pat, 1024, 1, out);
		/*printf ( "." ); */
	}
	/*printf ( " ok\n" ); */

	/* sample data */
	hio_seek(in, sdata_addr, SEEK_SET);
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_zen(uint8 *data, char *t, int s)
{
	int i;
	int len, pat_ofs;

	PW_REQUEST_DATA(s, 9 + 16 * 31);

	/* test #2 */
	pat_ofs = readmem32b(data);
	if (pat_ofs < 502 || pat_ofs > 2163190L)
		return -1;

	for (i = 0; i < 31; i++) {
		uint8 *d = data + 16 * i;
		if (d[9] > 0x40)
			return -1;

		/* finetune */
		if (readmem16b(d + 6) % 72)
			return -1;
	}

	/* smp sizes .. */
	for (i = 0; i < 31; i++) {
		int size = readmem16b(data + 10 + i * 16) << 1;
		int lsize = readmem16b(data + 12 + i * 16) << 1;
		int sdata = readmem32b(data + 14 + i * 16);

		/* sample size and loop size > 64k ? */
		if (size > 0xffff || lsize > 0xffff)
			return -1;

		/* sample address < pattern table address? */
		if (sdata < pat_ofs)
			return -1;

#if 0
		/* too big an address ? */
		if (sdata > in_size) {
			Test = BAD;
			return;
		}
#endif
	}

	/* test size of the pattern list */
	len = data[5];
	if (len == 0 || len > 0x7f)
		return -1;

	PW_REQUEST_DATA(s, pat_ofs + len * 4 + 4);

	/* test if the end of pattern list is $FFFFFFFF */
	if (readmem32b(data + pat_ofs + len * 4) != 0xffffffff)
		return -1;

	/* n is the highest address of a sample data */
	/* ssize is its size */

	pw_read_title(NULL, t, 0);

	return 0;
}

const struct pw_format pw_zen = {
	"Zen Packer",
	test_zen,
	depack_zen
};

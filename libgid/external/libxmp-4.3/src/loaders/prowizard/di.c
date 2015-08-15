/*
 * Digital_Illusion.c   Copyright (C) 1997 Asle / ReDoX
 *
 * Converts DI packed MODs back to PTK MODs
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_di(HIO_HANDLE *in, FILE *out)
{
	uint8 c1, c2, c3;
	uint8 note, ins, fxt, fxp;
	uint8 ptk_tab[5];
	uint8 nins, npat, max;
	uint8 ptable[128];
	uint16 paddr[128];
	uint8 tmp[50];
	int i, k;
	int seq_offs, /*pat_offs,*/ smp_offs;
	int size, ssize;
	int pos;

	memset(ptable, 0, 128);
	memset(ptk_tab, 0, 5);
	memset(paddr, 0, 128);

	pw_write_zero(out, 20);			/* title */

	nins = hio_read16b(in);
	seq_offs = hio_read32b(in);
	/*pat_offs =*/ hio_read32b(in);
	smp_offs = hio_read32b(in);

	ssize = 0;
	for (i = 0; i < nins; i++) {
		pw_write_zero(out, 22);			/* name */
		write16b(out, size = hio_read16b(in));	/* size */
		ssize += size * 2;
		write8(out, hio_read8(in));			/* finetune */
		write8(out, hio_read8(in));			/* volume */
		write16b(out, hio_read16b(in));		/* loop start */
		write16b(out, hio_read16b(in));		/* loop size */
	}

	memset(tmp, 0, 50);
	for (i = nins; i < 31; i++)
		fwrite(tmp, 30, 1, out);

	pos = hio_tell(in);
	hio_seek(in, seq_offs, SEEK_SET);

	i = 0;
	do {
		c1 = hio_read8(in);
		ptable[i++] = c1;
	} while (c1 != 0xff);

	ptable[i - 1] = 0;
	write8(out, npat = i - 1);

	write8(out, 0x7f);

	for (max = i = 0; i < 128; i++) {
		write8(out, ptable[i]);
		if (ptable[i] > max)
			max = ptable[i];
	}

	write32b(out, PW_MOD_MAGIC);

	hio_seek(in, pos, SEEK_SET);

	for (i = 0; i <= max; i++)
		paddr[i] = hio_read16b(in);

	for (i = 0; i <= max; i++) {
		hio_seek(in, paddr[i], 0);
		for (k = 0; k < 256; k++) {	/* 256 = 4 voices * 64 rows */
			memset(ptk_tab, 0, 5);
			c1 = hio_read8(in);
			if ((c1 & 0x80) == 0) {
				c2 = hio_read8(in);
				note = ((c1 << 4) & 0x30) | ((c2 >> 4) & 0x0f);
				ptk_tab[0] = ptk_table[note][0];
				ptk_tab[1] = ptk_table[note][1];
				ins = (c1 >> 2) & 0x1f;
				ptk_tab[0] |= (ins & 0xf0);
				ptk_tab[2] = (ins << 4) & 0xf0;
				fxt = c2 & 0x0f;
				ptk_tab[2] |= fxt;
				fxp = 0x00;
				ptk_tab[3] = fxp;
				fwrite (ptk_tab, 4, 1, out);
				continue;
			}
			if (c1 == 0xff) {
				memset(ptk_tab, 0, 5);
				fwrite (ptk_tab, 4, 1, out);
				continue;
			}
			c2 = hio_read8(in);
			c3 = hio_read8(in);
			note = (((c1 << 4) & 0x30) | ((c2 >> 4) & 0x0f));
			ptk_tab[0] = ptk_table[note][0];
			ptk_tab[1] = ptk_table[note][1];
			ins = (c1 >> 2) & 0x1f;
			ptk_tab[0] |= (ins & 0xf0);
			ptk_tab[2] = (ins << 4) & 0xf0;
			fxt = c2 & 0x0f;
			ptk_tab[2] |= fxt;
			fxp = c3;
			ptk_tab[3] = fxp;
			fwrite(ptk_tab, 4, 1, out);
		}
	}

	hio_seek(in, smp_offs, SEEK_SET);
	pw_move_data(out, in, ssize);

	return 0;
}


static int test_di(uint8 *data, char *t, int s)
{
	int i;
	int numsmp, ssize, psize;
	int ptab_offs, pat_offs, smp_offs;

	PW_REQUEST_DATA (s, 21);

#if 0
	/* test #1 */
	if (i < 17) {
		Test = BAD;
		return;
	}
#endif

	/* test #2  (number of sample) */
	numsmp = readmem16b(data);
	if (numsmp > 31)
		return -1;

	/* test #3 (finetunes and whole sample size) */
	ssize = 0;
	for (i = 0; i < numsmp; i++) {
		int len = readmem16b(data + 14) << 1;
		int start = readmem16b(data + 18) << 1;
		int lsize = readmem16b(data + 20) << 1;
		uint8 *d = data + i * 8;

		if (len > 0xffff || start > 0xffff || lsize > 0xffff)
			return -1;

		if (start + lsize > len)
			return -1;

		if (d[16] > 0x0f)
			return -1;

		if (d[17] > 0x40)
			return -1;

		/* get total size of samples */
		ssize += len;
	}
	if (ssize <= 2) {
		return -1;
	}

	/* test #4 (addresses of pattern in file ... ptk_tableible ?) */

	psize = numsmp * 8 + 2;

	ptab_offs = readmem32b(data + 2);	/* address of pattern table */
	pat_offs = readmem32b(data + 6);	/* address of pattern data */
	smp_offs = readmem32b(data + 10);	/* address of sample data */

	if (pat_offs <= ptab_offs || smp_offs <= ptab_offs || smp_offs <= pat_offs)
		return -1;

	if (pat_offs - ptab_offs > 128)
		return -1;

#if 0
	if (k > in_size || l > in_size || l > in_size)
		return -1;
#endif

	/* test #4,1 :) */
	if (ptab_offs < psize)
		return -1;

#if 0
	/* test #5 */
	if ((pat_offs + start) > in_size) {
		Test = BAD;
		return;
	}
#endif

	PW_REQUEST_DATA(s, pat_offs - 1);

	/* test pattern table reliability */
	for (i = ptab_offs; i < pat_offs - 1; i++) {
		if (data[i] > 0x80)
			return -1;
	}

	/* test #6  ($FF at the end of pattern list ?) */
	if (data[pat_offs - 1] != 0xff)
		return -1;

	/* test #7 (address of sample data > $FFFF ? ) */
	/* l is still the address of the sample data */
	if (smp_offs > 65535)
		return -1;

	pw_read_title(NULL, t, 0);

	return 0;
}

const struct pw_format pw_di = {
	"Digital Illusions",
	test_di,
	depack_di
};


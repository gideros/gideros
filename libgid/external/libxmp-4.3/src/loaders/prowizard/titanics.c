/*
 * TitanicsPlayer.c  Copyright (C) 2007 Sylvain "Asle" Chipaux
 *
 * Modified in 2009,2014 by Claudio Matsuoka
 */

/*
 * Titan Trax vol. 1: http://www.youtube.com/watch?v=blgm0EcPUd8
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


/* With the help of Xigh :) .. thx */
static int cmplong(const void *a, const void *b)
{
	return *(int *)a == *(int *)b ? 0 : *(int *)a > *(int *)b ? 1 : -1;
}


static int depack_titanics(HIO_HANDLE *in, FILE *out)
{
	uint8 buf[1024];
	long pat_addr[128];
	long pat_addr_ord[128];
	long pat_addr_final[128];
	long max = 0l;
	uint8 pat;
	uint32 smp_addr[15];
	uint16 smp_size[15];
	int i, j, k;

	for (i = 0; i < 128; i++)
		pat_addr[i] = pat_addr_ord[i] = pat_addr_final[i] = 0;

	pw_write_zero(out, 20);			/* write title */

	for (i = 0; i < 15; i++) {
		smp_addr[i] = hio_read32b(in);
		pw_write_zero(out, 22);		/* write name */
		write16b(out, smp_size[i] = hio_read16b(in));
		smp_size[i] *= 2;
		write8(out, hio_read8(in));		/* finetune */
		write8(out, hio_read8(in));		/* volume */
		write16b(out, hio_read16b(in));	/* loop start */
		write16b(out, hio_read16b(in));	/* loop size */
	}
	for (i = 15; i < 31; i++) {
		pw_write_zero(out, 22);		/* write name */
		write16b(out, 0);		/* sample size */
		write8(out, 0);			/* finetune */
		write8(out, 0x40);		/* volume */
		write16b(out, 0);		/* loop start */
		write16b(out, 1);		/* loop size */
	}

	/* pattern list */
	hio_read(buf, 2, 128, in);
	for (pat = 0; pat < 128; pat++) {
		if (buf[pat * 2] == 0xff)
			break;
		pat_addr_ord[pat] = pat_addr[pat] = readmem16b(buf + pat * 2);
	}

	write8(out, pat);		/* patterns */
	write8(out, 0x7f);		/* write ntk byte */

	/* With the help of Xigh :) .. thx */
	qsort(pat_addr_ord, pat, sizeof(long), cmplong);

	for (j = i = 0; i < pat; i++) {
		pat_addr_final[j++] = pat_addr_ord[i];
		while (pat_addr_ord[i + 1] == pat_addr_ord[i] && i < pat)
			i++;
	}

	memset(buf, 0, 128);

	/* write pattern list */
	for (i = 0; i < pat; i++) {
		for (j = 0; pat_addr[i] != pat_addr_final[j]; j++) ;
		buf[i] = j;
		if (j > max)
			max = j;
	}
	fwrite(buf, 128, 1, out);
	write32b(out, PW_MOD_MAGIC);	/* write M.K. */

	/* pattern data */
	for (i = 0; i <= max; i++) {
		uint8 x, y, c;
		int note;

		hio_seek(in, pat_addr_final[i], SEEK_SET);

		memset(buf, 0, 1024);
		x = hio_read8(in);

		for (k = 0; k < 64; ) {			/* row number */
			y = hio_read8(in);
			c = (y >> 6) * 4;		/* channel */

			note = y & 0x3f;

			if (note <= 36) {
				buf[k * 16 + c] = ptk_table[note][0];
				buf[k * 16 + c + 1] = ptk_table[note][1];
			}
			buf[k * 16 + c + 2] = hio_read8(in);
			buf[k * 16 + c + 3] = hio_read8(in);

			if (x & 0x80)
				break;

			/* next event */
			x = hio_read8(in);
			k += x & 0x7f;
		}

		fwrite(&buf[0], 1024, 1, out);
	}

	/* sample data */
	for (i = 0; i < 15; i++) {
		if (smp_addr[i]) {
			hio_seek(in, smp_addr[i], SEEK_SET);
			pw_move_data(out, in, smp_size[i]);
		}
	}

	return 0;
}

static int test_titanics(uint8 *data, char *t, int s)
{
	int i;
	int ssize;

	PW_REQUEST_DATA(s, 182);

	/* test samples */
	ssize = 0;
	for (i = 0; i < 15; i++) {
		int len, start, lsize;
		int addr;
		uint8 *d = data + i * 12;

		if (d[7] > 0x40)
			return -1;
			
		if (d[6] != 0)
			return -1;

		addr = readmem32b(d);
		if (/*addr > in_size ||*/ addr != 0 && addr < 180)
			return -1;

		len = readmem16b(d + 4);	/* size */
		start = readmem16b(d + 8);	/* loop start */
		lsize = readmem16b(d + 10);	/* loop size */

		if (start > len || lsize > (len + 1) || len > 32768)
			return -1;

		if (lsize == 0)
			return -1;

		if (len == 0 && (start != 0 || lsize != 1))
			return -1;

		ssize += len;
	}

	if (ssize < 2) {
		return -1;
	}

	/* test pattern addresses */
	{
		int addr = 0;

		for (i = 0; i < 256; i += 2) {
			addr = readmem16b(data + i + 180);
	
			if (addr == 0xffff)
				break;
	
			if (addr < 180)
				return -1;
		}
	
		if (addr != 0xffff) {
			return -1;
		}
	}

	pw_read_title(NULL, t, 0);

	return 0;
}

const struct pw_format pw_titanics = {
	"Titanics Player",
	test_titanics,
	depack_titanics
};

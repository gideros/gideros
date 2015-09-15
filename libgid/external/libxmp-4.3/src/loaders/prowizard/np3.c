/*
 * NoisePacker_v3.c   Copyright (C) 1998 Asle / ReDoX
 *
 * Converts NoisePacked MODs back to ptk
 * Last revision : 26/11/1999 by Sylvain "Asle" Chipaux
 *                 reduced to only one FREAD.
 *                 Speed-up and Binary smaller.
 *
 * Modified in 2006,2007,2014,2015 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_np3(HIO_HANDLE *in, FILE *out)
{
	uint8 tmp[1024];
	uint8 c1, c2, c3, c4;
	uint8 ptable[128];
	int len, nins, npat;
	int max_addr, smp_addr;
	int size, ssize = 0;
	/*int tsize;*/
	int trk_addr[128][4];
	int i, j, k;
	int trk_start;

	memset(ptable, 0, 128);
	memset(trk_addr, 0, 128 * 4 * 4);

	c1 = hio_read8(in);			/* read number of samples */
	c2 = hio_read8(in);
	nins = ((c1 << 4) & 0xf0) | ((c2 >> 4) & 0x0f);

	pw_write_zero(out, 20);			/* write title */

	len = hio_read16b(in) >> 1;		/* size of pattern list */
	hio_read16b(in);			/* 2 unknown bytes */
	/*tsize =*/ hio_read16b(in);		/* read track data size */

	/* read sample descriptions */
	for (i = 0; i < nins; i++) {
		hio_read(tmp, 1, 16, in);
		pw_write_zero(out, 22);		/* sample name */
		write16b(out, size = readmem16b(tmp + 6));
		ssize += size * 2;
		write8(out, tmp[0]);		/* write finetune */
		write8(out, tmp[1]);		/* write volume */
		fwrite(tmp + 14, 2, 1, out);	/* write loop start */
		fwrite(tmp + 12, 2, 1, out);	/* write loop size */
	}

	/* fill up to 31 samples */
	memset(tmp, 0, 30);
	tmp[29] = 0x01;
	for (; i < 31; i++)
		fwrite(tmp, 30, 1, out);

	write8(out, len);		/* write size of pattern list */
	write8(out, 0x7f);		/* write noisetracker byte */

	hio_seek(in, 2, SEEK_CUR);	/* always $02? */
	hio_seek(in, 2, SEEK_CUR);	/* unknown */

	/* read pattern table */
	npat = 0;
	for (i = 0; i < len; i++) {
		ptable[i] = hio_read16b(in) / 8;
		if (ptable[i] > npat)
			npat = ptable[i];
	}
	npat++;

	fwrite(ptable, 128, 1, out);	/* write pattern table */
	write32b(out, PW_MOD_MAGIC);	/* write ptk ID */

	/* read tracks addresses per pattern */
	for (max_addr = i = 0; i < npat; i++) {
		if ((trk_addr[i][0] = hio_read16b(in)) > max_addr)
			max_addr = trk_addr[i][0];
		if ((trk_addr[i][1] = hio_read16b(in)) > max_addr)
			max_addr = trk_addr[i][1];
		if ((trk_addr[i][2] = hio_read16b(in)) > max_addr)
			max_addr = trk_addr[i][2];
		if ((trk_addr[i][3] = hio_read16b(in)) > max_addr)
			max_addr = trk_addr[i][3];
	}
	trk_start = hio_tell(in);

	/* the track data now ... */
	smp_addr = 0;
	for (i = 0; i < npat; i++) {
		memset(tmp, 0, 1024);
		for (j = 0; j < 4; j++) {
			hio_seek(in, trk_start + trk_addr[i][3 - j], SEEK_SET);
			for (k = 0; k < 64; k++) {
				int x = k * 16 + j * 4;

				if ((c1 = hio_read8(in)) >= 0x80) {
					k += (0x100 - c1) - 1;
					continue;
				}
				c2 = hio_read8(in);
				c3 = hio_read8(in);
				c4 = (c1 & 0xfe) / 2;

				tmp[x] = ((c1 << 4) & 0x10) | ptk_table[c4][0];
				tmp[x + 1] = ptk_table[c4][1];

				switch (c2 & 0x0f) {
				case 0x08:
					c2 &= 0xf0;
					break;
				case 0x07:
					c2 = (c2 & 0xf0) + 0x0a;
					/* fall through */
				case 0x06:
				case 0x05:
					c3 = c3 > 0x80 ? 0x100 - c3 :
							(c3 << 4) & 0xf0;
					break;
				case 0x0e:
					c3 = 1;
					break;
				case 0x0b:
					c3 = (c3 + 4) / 2;
					break;
				}

				tmp[x + 2] = c2;
				tmp[x + 3] = c3;

				if ((c2 & 0x0f) == 0x0d)
					break;
			}

			if (hio_tell(in) > smp_addr)
				smp_addr = hio_tell(in);
		}
		fwrite(tmp, 1024, 1, out);
	}

	/* sample data */
	if (smp_addr & 1)
		smp_addr++;
	hio_seek(in, smp_addr, SEEK_SET);
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_np3(uint8 *data, char *t, int s)
{
	int num_ins, ssize, hdr_size, ptab_size, trk_size, max_pptr;
	int i;

	PW_REQUEST_DATA(s, 10);

	/* size of the pattern table */
	ptab_size = readmem16b(data + 2);
	if (ptab_size == 0 || ptab_size & 0x01 || ptab_size > 0xff)
		return -1;

	/* test number of samples */
	if ((data[1] & 0x0f) != 0x0c)
		return -1;

	num_ins = ((data[0] << 4) & 0xf0) | ((data[1] >> 4) & 0x0f);
	if (num_ins == 0 || num_ins > 0x1f)
		return -1;

	PW_REQUEST_DATA(s, 15 + num_ins * 16);

	/* test volumes */
	for (i = 0; i < num_ins; i++) {
		if (data[9 + i * 16] > 0x40)
			return -1;
	}

	/* test sample sizes */
	ssize = 0;
	for (i = 0; i < num_ins; i++) {
		uint8 *d = data + i * 16;
		int len = readmem16b(d + 14) << 1;
		int start = readmem16b(d + 20) << 1;
		int lsize = readmem16b(d + 22) << 1;

		if (len > 0xffff || start > 0xffff || lsize > 0xffff)
			return -1;

		if (start + lsize > len + 2)
			return -1;

		if (start == 0 && lsize != 0)
			return -1;

		ssize += len;
	}

	if (ssize <= 4)
		return -1;

	/* size of the header 'til the end of sample descriptions */
	hdr_size = num_ins * 16 + 8 + 4;

	PW_REQUEST_DATA(s, hdr_size + ptab_size + 2);

	/* test pattern table */
	max_pptr = 0;
	for (i = 0; i < ptab_size; i += 2) {
		int pptr = readmem16b(data + hdr_size + i);
		if (pptr & 0x07 || pptr > 0x400)
			return -1;
		if (pptr > max_pptr)
			max_pptr = pptr;
	}

	/* max_pptr is the highest pattern number (*8) */

	/* paske on a que l'address du dernier pattern .. */
	/* size of the header 'til the end of the track list */
	hdr_size += ptab_size + max_pptr + 8;

	/* test track data size */
	trk_size = readmem16b(data + 6);
	if (trk_size <= 63)
		return -1;

	PW_REQUEST_DATA(s, hdr_size + trk_size + 2);

	/* test notes */
	/* re-calculate the number of sample */
	for (i = 0; i < trk_size ; i++) {
		uint8 *d = data + hdr_size + i;

		if (d[0] & 0x80)
			continue;

		/* si note trop grande et si effet = A */
		if (d[0] > 0x49 || (d[1] & 0x0f) == 0x0a)
			return -1;

		/* si effet D et arg > 0x40 */
		if ((d[1] & 0x0f) == 0x0d && d[2] > 0x40)
			return -1;

		/* sample nbr > ce qui est defini au debut ? */
		if ((((d[0] << 4) & 0x10) | ((d[1] >> 4) & 0x0f)) > num_ins)
			return -1;

		/* all is empty ?!? ... cannot be ! */
		if (d[0] == 0 && d[1] == 0 && d[2] == 0 && i < (trk_size - 3))
			return -1;

		i += 2;
	}

	pw_read_title(NULL, t, 0);

	return 0;
}

const struct pw_format pw_np3 = {
	"NoisePacker v3",
	test_np3,
	depack_np3
};

/*
 * Unic_Tracker.c   Copyright (C) 1997 Asle / ReDoX
 * 
 * Unic tracked MODs to Protracker
 * both with or without ID Unic files will be converted
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"

#define MAGIC_UNIC	MAGIC4('U','N','I','C')
#define MAGIC_M_K_	MAGIC4('M','.','K','.')
#define MAGIC_0000	MAGIC4(0x0,0x0,0x0,0x0)


static int depack_unic(HIO_HANDLE *in, FILE *out)
{
	uint8 c1, c2, c3, c4;
	uint8 npat;
	uint8 max = 0;
	uint8 ins, note, fxt, fxp;
	uint8 fine;
	uint8 tmp[1025];
	int i, j;
	int ssize;
	uint32 id;

	pw_move_data(out, in, 20);		/* title */

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
		ssize += len * 2;

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

	npat = hio_read8(in);
	write8(out, npat);			/* number of pattern */
	write8(out, 0x7f);			/* noisetracker byte */
	hio_read8(in);

	hio_read(tmp, 128, 1, in);			/* pat table */
	fwrite(tmp, 128, 1, out);

	/* get highest pattern number */
	for (i = 0; i < 128; i++) {
		if (tmp[i] > max)
			max = tmp[i];
	}
	max++;		/* coz first is $00 */

	write32b(out, PW_MOD_MAGIC);

	/* verify UNIC ID */
	hio_seek(in, 1080, SEEK_SET);
	id = hio_read32b(in);

	if (id && id != MAGIC_M_K_ && id != MAGIC_UNIC)
		hio_seek(in, -4, SEEK_CUR);

	/* pattern data */
	for (i = 0; i < max; i++) {
		for (j = 0; j < 256; j++) {
			c1 = hio_read8(in);
			c2 = hio_read8(in);
			c3 = hio_read8(in);

			ins = ((c1 >> 2) & 0x10) | ((c2 >> 4) & 0x0f);
			note = c1 & 0x3f;
			fxt = c2 & 0x0f;
			fxp = c3;

			if (fxt == 0x0d) {	/* pattern break */
				c3 = fxp / 10;
				c4 = fxp % 10;
				fxp = 16 * c3 + c4;
			}

			tmp[j * 4] = (ins & 0xf0) | ptk_table[note][0];
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

static int check_instruments(uint8 *data)
{
	int ssize, max_ins;
	int i;

	ssize = 0;
	max_ins = 0;
	for (i = 0; i < 31; i++) {
		uint8 *d = data + i * 30;
		int len = readmem16b(d + 42) << 1;
		int start = readmem16b(d + 46) << 1;
		int lsize = readmem16b(d + 48) << 1;
		int fine;

		ssize += len;
		if (lsize != 0 && (len + 2) < (start + lsize))
			return -1;

		/* samples too big ? */
		if (len > 0xffff || start > 0xffff || lsize > 0xffff)
			return -1;

		/* volume too big */
		if (d[45] > 0x40)
			return -1;

		/* finetune ... */
		fine = readmem16b(d + 40);
		if ((fine != 0 && len == 0) || (fine > 8 && fine < 247))
			return -1;

		/* loop start but no replen ? */
		if (start != 0 && lsize <= 2)
			return -1;

		if (d[45] != 0 && len == 0)
			return -1;

		/* get the highest !0 sample */
		if (len != 0)
			max_ins = i + 1;
	}
	if (ssize <= 2) {
		return -1;
	}

	return max_ins;
}

static int check_pattern_list_size(uint8 *data)
{
	int len, psize;
	int i;

	/* test #4  pattern list size */
	len = data[950];
	if (len == 0 || len > 127)
		return -1;

	psize = 0;
	for (i = 0; i < len; i++) {
		int x = data[952 + i];
		if (x > 127)
			return -1;
		if (x > psize)
			psize = x;
	}

	/* test last patterns of the pattern list = 0 ? */
	for (; i != 128; i++) {
		if (data[952 + i] != 0)
			return -1;
	}

	psize++;
	psize <<= 8;

	return psize;
}

static int check_pattern(uint8 *data, int s, int psize, int max_ins, int offset)
{
	int i;

	PW_REQUEST_DATA(s, offset + psize * 3 + 2);

	for (i = 0; i < psize; i++) {
		uint8 *d = data + offset + i * 3;
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

		ins = ((d[0] >> 2) & 0x30) | ((d[2] >> 4) & 0x0F);

		if (ins > max_ins)
			return -1;
	}

	return 0;
}

static int test_unic_id(uint8 *data, char *t, int s)
{
	int i;
	int psize, ssize;

	/* test 1 */
	PW_REQUEST_DATA(s, 1084);

	if (readmem32b(data + 1080) != MAGIC_M_K_)
		return -1;

	/* test 2 */
	ssize = 0;
	for (i = 0; i < 31; i++) {
		uint8 *d = data + i * 30;
		int size, end;

		size = readmem16b(d + 42) << 1;
		ssize += size;
		end = (readmem16b(d + 46) + readmem16b(d + 48)) << 1;

		if ((size + 2) < end)
			return -1;
	}

	if (ssize <= 2)
		return -1;

	/* test #3  finetunes & volumes */
	for (i = 0; i < 31; i++) {
		uint8 *d = data + i * 30;
		if ((int8)d[40] < -8 || (int8)d[40] > 7)
			return -1;
		if (d[44] != 0 || d[45] > 0x40)
			return -1;
	}

	/* test #4  pattern list size */
	psize = check_pattern_list_size(data);
	if (psize < 0)
		return -1;

	/* test #5 pattern data ... */
	for (i = 0; i < psize; i++) {
		/* relative note number + last bit of sample > $34 ? */
		if (data[1084 + i * 3] > 0x74)
			return -1;
	}

	pw_read_title(data, t, 20);

	return 0;
}

static int test_unic_emptyid(uint8 *data, char *t, int s)
{
	int psize, max_ins;

	/* test 1 */
	PW_REQUEST_DATA(s, 1084);

	/* test #2 ID = $00000000 ? */
	if (readmem32b(data + 1080) != MAGIC_0000)
		return -1;

	/* test 2,5 :) */
	max_ins = check_instruments(data);
	if (max_ins < 0)
		return -1;

	/* test #4  pattern list size */
	psize = check_pattern_list_size(data);
	if (psize < 0)
		return -1;

	/* test #5 pattern data ... */
	if (check_pattern(data, s, psize, max_ins, 1084) < 0)
		return -1;

	pw_read_title(data, t, 20);

	return 0;
}

static int test_unic_noid(uint8 *data, char *t, int s)
{
	int i;
	int psize, max_ins;

	/* test 1 */
	PW_REQUEST_DATA(s, 1084);

	/* test #2 ID = $00000000 ? */
	if (readmem32b(data + 1080) == MAGIC_0000)
		return -1;

	/* test 2,5 :) */
	max_ins = check_instruments(data);
	if (max_ins < 0)
		return -1;

	/* test #4  pattern list size */
	psize = check_pattern_list_size(data);
	if (psize < 0)
		return -1;

	/* test #5 pattern data ... */
	if (check_pattern(data, s, psize, max_ins, 1080) < 0)
		return -1;

	/* test #6  title coherent ? */
	for (i = 0; i < 20; i++) {
		if ((data[i] != 0 && data[i] < 32) ||
			data[i] > 180)
			return -1;
	}

	pw_read_title(data, t, 20);

	return 0;
}

const struct pw_format pw_unic_id = {
	"UNIC Tracker",
	test_unic_id,
	depack_unic
};

const struct pw_format pw_unic_noid = {
	"UNIC Tracker noid",
	test_unic_noid,
	depack_unic
};

const struct pw_format pw_unic_emptyid = {
	"UNIC Tracker id0",
	test_unic_emptyid,
	depack_unic
};

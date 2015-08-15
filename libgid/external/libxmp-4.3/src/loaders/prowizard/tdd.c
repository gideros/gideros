/*
 * tdd.c   Copyright (C) 1999 Asle / ReDoX
 *
 * Converts TDD packed MODs back to PTK MODs
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_tdd(HIO_HANDLE *in, FILE *out)
{
	uint8 *tmp;
	uint8 pat[1024];
	uint8 pmax;
	int i, j, k;
	int size, ssize = 0;
	int saddr[31];
	int ssizes[31];

	memset(saddr, 0, 31 * 4);
	memset(ssizes, 0, 31 * 4);

	/* write ptk header */
	pw_write_zero(out, 1080);

	/* read/write pattern list + size and ntk byte */
	tmp = (uint8 *)malloc(130);
	memset(tmp, 0, 130);
	fseek(out, 950, 0);
	hio_read(tmp, 130, 1, in);
	fwrite(tmp, 130, 1, out);

	for (pmax = i = 0; i < 128; i++) {
		if (tmp[i + 2] > pmax) {
			pmax = tmp[i + 2];
		}
	}
	free(tmp);

	/* sample descriptions */
	for (i = 0; i < 31; i++) {
		fseek(out, 42 + (i * 30), SEEK_SET);
		/* sample address */
		saddr[i] = hio_read32b(in);

		/* read/write size */
		write16b(out, size = hio_read16b(in));
		ssize += size;
		ssizes[i] = size;

		write8(out, hio_read8(in));		/* read/write finetune */
		write8(out, hio_read8(in));		/* read/write volume */
		/* read/write loop start */
		write16b(out, (hio_read32b(in) - saddr[i]) / 2);
		write16b(out, hio_read16b(in));	/* read/write replen */
	}

	/* bypass Samples datas */
	hio_seek(in, ssize, SEEK_CUR);

	/* write ptk's ID string */
	fseek(out, 0, SEEK_END);
	write32b(out, PW_MOD_MAGIC);

	/* read/write pattern data */
	tmp = (uint8 *)malloc(1024);
	for (i = 0; i <= pmax; i++) {
		memset(tmp, 0, 1024);
		memset(pat, 0, 1024);
		hio_read(tmp, 1024, 1, in);
		for (j = 0; j < 64; j++) {
			for (k = 0; k < 4; k++) {
				int x = j * 16 + k * 4;

				/* fx arg */
				pat[x + 3] = tmp[x + 3];

				/* fx */
				pat[x + 2] = tmp[x + 2] & 0x0f;

				/* smp */
				pat[x] = tmp[x] & 0xf0;
				pat[x + 2] |= (tmp[x] << 4) & 0xf0;

				/* note */
				pat[x] |= ptk_table[tmp[x + 1] / 2][0];
				pat[x + 1] = ptk_table[tmp[x + 1] / 2][1];
			}
		}
		fwrite(pat, 1024, 1, out);
	}
	free(tmp);

	/* Sample data */
	for (i = 0; i < 31; i++) {
		if (ssizes[i] == 0)
			continue;
		hio_seek(in, saddr[i], SEEK_SET);
		pw_move_data(out, in, ssizes[i]);
	}

	return 0;
}

static int test_tdd(uint8 *data, char *t, int s)
{
	int i;
	int ssize, psize, pdata_ofs;

	PW_REQUEST_DATA(s, 564);

	/* test #2 (volumes,sample addresses and whole sample size) */
	ssize = 0;
	for (i = 0; i < 31; i++) {
		uint8 *d = data + i * 14;
		int addr = readmem32b(d + 130);	/* sample address */
		int size = readmem16b(d + 134);	/* sample size */
		int sadr = readmem32b(d + 138);	/* loop start address */
		int lsiz = readmem16b(d + 142);	/* loop size (replen) */

		/* volume > 40h ? */
		if (d[137] > 0x40)
			return -1;

		/* loop start addy < sampl addy ? */
		if (sadr < addr)
			return -1;

		/* addy < 564 ? */
		if (addr < 564 || sadr < 564)
			return -1;

		/* loop start > size ? */
		if (sadr - addr > size)
			return -1;

		/* loop start+replen > size ? */
		if (sadr - addr + lsiz > size + 2)
			return -1;

		ssize += size;
	}

	if (ssize <= 2 || ssize > 31 * 65535)
		return -1;

#if 0
	/* test #3 (addresses of pattern in file ... ptk_tableible ?) */
	/* ssize is the whole sample size :) */
	if ((ssize + 564) > in_size) {
		Test = BAD;
		return;
	}
#endif

	/* test size of pattern list */
	if (data[0] == 0 || data[0] > 0x7f)
		return -1;

	/* test pattern list */
	psize = 0;
	for (i = 0; i < 128; i++) {
		int pat = data[i + 2];
		if (pat > 0x7f)
			return -1;
		if (pat > psize)
			psize = pat;
	}
	psize++;
	psize <<= 10;

	/* test end of pattern list */
	for (i = data[0] + 2; i < 128; i++) {
		if (data[i + 2] != 0)
			return -1;
	}

#if 0
	/* test if not out of file range */
	if ((ssize + 564 + k) > in_size)
		return -1;
#endif

	/* ssize is the whole sample data size */
	/* test pattern data now ... */
	pdata_ofs = 564 + ssize;

	PW_REQUEST_DATA(s, 564 + ssize + psize);

	for (i = 0; i < psize; i += 4) {
		uint8 *d = data + pdata_ofs + i;

		/* sample number > 31 ? */
		if (d[0] > 0x1f)
			return -1;

		/* note > 0x48 (36*2) */
		if (d[1] > 0x48 || (d[1] & 0x01) == 0x01)
			return -1;

		/* fx=C and fxtArg > 64 ? */
		if ((d[2] & 0x0f) == 0x0c && d[3] > 0x40)
			return -1;

		/* fx=D and fxtArg > 64 ? */
		if ((d[2] & 0x0f) == 0x0d && d[3] > 0x40)
			return -1;

		/* fx=B and fxtArg > 127 ? */
		if ((d[2] & 0x0f) == 0x0b)
			return -1;
	}

	pw_read_title(NULL, t, 0);

	return -1;
}

const struct pw_format pw_tdd = {
	"The Dark Demon",
	test_tdd,
	depack_tdd
};

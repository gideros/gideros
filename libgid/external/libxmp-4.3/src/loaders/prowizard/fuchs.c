/*
 * FuchsTracker.c   Copyright (C) 1999 Sylvain "Asle" Chipaux
 *
 * Depacks Fuchs Tracker modules
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_fuchs(HIO_HANDLE *in, FILE *out)
{
	uint8 *tmp;
	uint8 max_pat;
	/*int ssize;*/
	int smp_len[16];
	int loop_start[16];
	int pat_size;
	int i;

	memset(smp_len, 0, 16 * 4);
	memset(loop_start, 0, 16 * 4);

	pw_write_zero(out, 1080);		/* write ptk header */
	fseek(out, 0, SEEK_SET);
	pw_move_data(out, in, 10);		/* read/write title */
	/*ssize =*/ hio_read32b(in);		/* read all sample data size */

	/* read/write sample sizes */
	for (i = 0; i < 16; i++) {
		fseek(out, 42 + i * 30, SEEK_SET);
		smp_len[i] = hio_read16b(in);
		write16b(out, smp_len[i] / 2);
	}

	/* read/write volumes */
	for (i = 0; i < 16; i++) {
		fseek(out, 45 + i * 30, SEEK_SET);
		hio_seek(in, 1, SEEK_CUR);
		write8(out, hio_read8(in));
	}

	/* read/write loop start */
	for (i = 0; i < 16; i++) {
		fseek(out, 46 + i * 30, SEEK_SET);
		loop_start[i] = hio_read16b(in);
		write8(out, loop_start[i] / 2);
	}

	/* write replen */
	for (i = 0; i < 16; i++) {
		int loop_size;

		fseek(out, 48 + i * 30, SEEK_SET);
		loop_size = smp_len[i] - loop_start[i];
		if (loop_size == 0 || loop_start[i] == 0) {
			write16b(out, 0x0001);
		} else {
			write16b(out, loop_size / 2);
		}
	}

	/* fill replens up to 31st sample wiz $0001 */
	for (i = 16; i < 31; i++) {
		fseek(out, 48 + i * 30, SEEK_SET);
		write16b(out, 0x0001);
	}

	/* that's it for the samples ! */
	/* now, the pattern list */

	/* read number of pattern to play */
	fseek(out, 950, SEEK_SET);
	/* bypass empty byte (saved wiz a WORD ..) */
	hio_seek(in, 1, SEEK_CUR);
	write8(out, hio_read8(in));

	/* write ntk byte */
	write8(out, 0x7f);

	/* read/write pattern list */
	for (max_pat = i = 0; i < 40; i++) {
		uint8 pat;
		hio_seek(in, 1, SEEK_CUR);
		pat = hio_read8(in);
		write8(out, pat);
		if (pat > max_pat)
			max_pat = pat;
	}

	/* write ptk's ID */
	fseek(out, 0, SEEK_END);
	write32b(out, PW_MOD_MAGIC);

	/* now, the pattern data */

	/* bypass the "SONG" ID */
	hio_seek(in, 4, 1);

	/* read pattern data size */
	pat_size = hio_read32b(in);

	/* Sanity check */
	if (pat_size <= 0 || pat_size > 0x20000)
		return -1;

	/* read pattern data */
	tmp = (uint8 *)malloc(pat_size);
	hio_read(tmp, pat_size, 1, in);

	/* convert shits */
	for (i = 0; i < pat_size; i += 4) {
		/* convert fx C arg back to hex value */
		if ((tmp[i + 2] & 0x0f) == 0x0c) {
			int x = tmp[i + 3];
			tmp[i + 3] = 10 * (x >> 4) + (x & 0xf);
		}
	}

	/* write pattern data */
	fwrite(tmp, pat_size, 1, out);
	free(tmp);

	/* read/write sample data */
	hio_seek(in, 4, SEEK_CUR);	/* bypass "INST" Id */
	for (i = 0; i < 16; i++) {
		if (smp_len[i] != 0)
			pw_move_data(out, in, smp_len[i]);
	}

	return 0;
}

static int test_fuchs (uint8 *data, char *t, int s)
{
	int i;
	int ssize, hdr_ssize;

#if 0
	/* test #1 */
	if (i < 192) {
		Test = BAD;
		return;
	}
	start = i - 192;
#endif

	if (readmem32b(data + 192) != 0x534f4e47)	/* SONG */
		return -1;

	/* all sample size */
	hdr_ssize = readmem32b(data + 10);

	if (hdr_ssize <= 2 || hdr_ssize >= 65535 * 16)
		return -1;

	/* samples descriptions */
	ssize = 0;
	for (i = 0; i < 16; i++) {
		uint8 *d = data + i * 2;
		int len = readmem16b(d + 14);
		int start = readmem16b(d + 78);

		/* volumes */
		if (d[46] > 0x40)
			return -1;

		if (len < start)
			return -1;

		ssize += len;
	}

	if (ssize <= 2 || ssize > hdr_ssize)
		return -1;

	/* get highest pattern number in pattern list */
	/*max_pat = 0;*/
	for (i = 0; i < 40; i++) {
		int pat = data[i * 2 + 113];
		if (pat > 40)
			return -1;
		/*if (pat > max_pat)
			max_pat = pat;*/
	}

#if 0
	/* input file not long enough ? */
	max_pat++;
	max_pat *= 1024;
	PW_REQUEST_DATA (s, k + 200);
#endif

	pw_read_title(NULL, t, 0);

	return 0;
}

const struct pw_format pw_fchs = {
	"Fuchs Tracker",
	test_fuchs,
	depack_fuchs
};


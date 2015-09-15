/*
 * fuzzac.c   Copyright (C) 1997 Asle / ReDoX
 *
 * Converts Fuzzac packed MODs back to PTK MODs
 * thanks to Gryzor and his ProWizard tool ! ... without it, this prog
 * would not exist !!!
 *
 * note: A most worked-up prog ... took some time to finish this !.
 *      there's what lot of my other depacker are missing : the correct
 *      pattern order (most of the time the list is generated badly ..).
 *      Dont know why I did it for this depacker because I've but one
 *      exemple file ! :)
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_fuzz(HIO_HANDLE *in, FILE *out)
{
	uint8 c1;
	uint8 data[1024];
	uint8 ord[128];
	uint8 tidx[128][16];
	uint8 tidx_real[128][4];
	uint8 track[4][256];
	uint8 status = 1;
	int len, ntrk, npat;
	int size, ssize = 0;
	int lps, lsz;
	int i, j, k, l;

	memset(tidx, 0, 128 * 16);
	memset(tidx_real, 0, 128 * 4);
	memset(ord, 0, 128);

	hio_read32b(in);			/* bypass ID */
	hio_read16b(in);			/* bypass 2 unknown bytes */
	pw_write_zero(out, 20);		/* write title */

	for (i = 0; i < 31; i++) {
		pw_move_data(out, in, 22);	/*sample name */
		hio_seek(in, 38, SEEK_CUR);
		write16b(out, size = hio_read16b(in));
		ssize += size * 2;
		lps = hio_read16b(in);		/* loop start */
		lsz = hio_read16b(in);		/* loop size */
		write8(out, hio_read8(in));		/* finetune */
		write8(out, hio_read8(in));		/* volume */
		write16b(out, lps);
		write16b(out, lsz > 0 ? lsz : 1);
	}

	len = hio_read8(in);		/* size of pattern list */

	/* Sanity check */
	if (len > 128)
		return -1;

	write8(out, len);
	ntrk = hio_read8(in);		/* read the number of tracks */
	write8(out, 0x7f);		/* write noisetracker byte */

	/* place file pointer at track number list address */
	hio_seek(in, 2118, SEEK_SET);

	/* read tracks numbers */
	for (i = 0; i < 4; i++) {
		for (j = 0; j < len; j++)
			hio_read(&tidx[j][i * 4], 1, 4, in);
	}

	/* sort tracks numbers */
	npat = 0;
	for (i = 0; i < len; i++) {
		if (i == 0) {
			ord[0] = npat++;
			continue;
		}

		for (j = 0; j < i; j++) {
			status = 1;
			for (k = 0; k < 4; k++) {
				if (tidx[j][k * 4] !=
					tidx[i][k * 4]) {
					status = 0;
					break;
				}
			}
			if (status == 1) {
				ord[i] = ord[j];
				break;
			}
		}

		if (status == 0)
			ord[i] = npat++;

		status = 1;
	}

	/* create a list of tracks numbers for the really existing patterns */
	c1 = 0x00;
	for (i = 0; i < len; i++) {
		if (i == 0) {
			tidx_real[c1][0] = tidx[i][0];
			tidx_real[c1][1] = tidx[i][4];
			tidx_real[c1][2] = tidx[i][8];
			tidx_real[c1][3] = tidx[i][12];
			c1++;
			continue;
		}

		for (j = 0; j < i; j++) {
			status = 1;
			if (ord[i] == ord[j]) {
				status = 0;
				break;
			}
		}

		if (status == 0)
			continue;

		tidx_real[c1][0] = tidx[i][0];
		tidx_real[c1][1] = tidx[i][4];
		tidx_real[c1][2] = tidx[i][8];
		tidx_real[c1][3] = tidx[i][12];
		c1++;
		status = 1;
	}

	fwrite(ord, 128, 1, out);	/* write pattern list */
	write32b(out, PW_MOD_MAGIC);	/* write ID */

	/* pattern data */
	l = 2118 + len * 16;

	for (i = 0; i < npat; i++) {
		memset(data, 0, 1024);
		memset(track, 0, 4 << 8);

		hio_seek(in, l + (tidx_real[i][0] << 8), SEEK_SET);
		hio_read(track[0], 256, 1, in);

		hio_seek(in, l + (tidx_real[i][1] << 8), SEEK_SET);
		hio_read(track[1], 256, 1, in);

		hio_seek(in, l + (tidx_real[i][2] << 8), SEEK_SET);
		hio_read(track[2], 256, 1, in);

		hio_seek(in, l + (tidx_real[i][3] << 8), SEEK_SET);
		hio_read(track[3], 256, 1, in);

		for (j = 0; j < 64; j++) {
			memcpy(&data[j * 16     ], &track[0][j * 4], 4);
			memcpy(&data[j * 16 + 4 ], &track[1][j * 4], 4);
			memcpy(&data[j * 16 + 8 ], &track[2][j * 4], 4);
			memcpy(&data[j * 16 + 12], &track[3][j * 4], 4);
			data[j * 16 + 15] = track[3][j * 4 + 3];
		}
		fwrite(data, 1024, 1, out);
	}

	/* sample data */
	/* bypass the "SEnd" unidentified ID */
	hio_seek(in, l + (ntrk << 8) + 4, SEEK_SET);
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_fuzz(uint8 *data, char *t, int s)
{
	int i;

	if (readmem32b(data) != MAGIC4('M','1','.','0'))
		return -1;

	/* test finetune */
	for (i = 0; i < 31; i++) {
		if (data[72 + i * 68] > 0x0f)
			return -1;
	}

	/* test volumes */
	for (i = 0; i < 31; i++) {
		if (data[73 + i * 68] > 0x40)
			return -1;
	}

	/* test sample sizes */
	for (i = 0; i < 31; i++) {
		int len = readmem16b(data + i * 68 + 66);
		if (len > 0x8000)
			return -1;
	}

	/* test size of pattern list */
	if (data[2114] == 0x00)
		return -1;

	pw_read_title(NULL, t, 0);

	return 0;
}

const struct pw_format pw_fuzz = {
	"Fuzzac Packer",
	test_fuzz,
	depack_fuzz
};


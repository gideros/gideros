/*
 * Kefrens_Sound_Machine.c   Copyright (C) 1997 Sylvain "Asle" Chipaux
 *
 * Depacks musics in the Kefrens Sound Machine format and saves in ptk.
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_ksm(HIO_HANDLE *in, FILE *out)
{
	uint8 tmp[1024];
	uint8 c1, c5;
	uint8 plist[128];
	uint8 trknum[128][4];
	uint8 real_tnum[128][4];
	uint8 tdata[4][192];
	uint8 max_trknum;
	uint8 len;
	uint8 status = 1;
	int ssize = 0;
	int i, j, k;

	memset(plist, 0, 128);
	memset(trknum, 0, 128 * 4);
	memset(real_tnum, 0, 128 * 4);

	/* title */
	hio_seek(in, 2, SEEK_SET);
	pw_move_data(out, in, 13);
	pw_write_zero(out, 7);

	/* read and write whole header */
	/*printf ( "Converting sample headers ... " ); */
	hio_seek(in, 32, SEEK_SET);
	for (i = 0; i < 15; i++) {
		pw_write_zero(out, 22);		/* write name */
		hio_seek(in, 20, SEEK_CUR);	/* 16 unknown/4 addr bytes */
		write16b(out, (k = hio_read16b(in)) / 2); /* size */
		ssize += k;
		write8(out, 0);			/* finetune */
		write8(out, hio_read8(in));		/* volume */
		hio_read8(in);			/* bypass 1 unknown byte */
		write16b(out, (j = hio_read16b(in)) / 2);	/* loop start */
		j = k - j;
		write16b(out, j != k ? j / 2 : 1);	/* loop size */
		hio_seek(in, 6, SEEK_CUR);		/* bypass 6 unknown bytes */
	}

	memset(tmp, 0, 30);
	tmp[29] = 1;
	for (i = 0; i < 16; i++)
		fwrite(tmp, 30, 1, out);

	/* pattern list */
	hio_seek(in, 512, SEEK_SET);
	for (max_trknum = len = 0; len < 128; len++) {
		hio_read(&trknum[len][0], 1, 1, in);
		hio_read(&trknum[len][1], 1, 1, in);
		hio_read(&trknum[len][2], 1, 1, in);
		hio_read(&trknum[len][3], 1, 1, in);
		if (trknum[len][0] == 0xFF)
			break;
		if (trknum[len][0] > max_trknum)
			max_trknum = trknum[len][0];
		if (trknum[len][1] > max_trknum)
			max_trknum = trknum[len][1];
		if (trknum[len][2] > max_trknum)
			max_trknum = trknum[len][2];
		if (trknum[len][3] > max_trknum)
			max_trknum = trknum[len][3];
	}

	write8(out, len);		/* write patpos */
	write8(out, 0x7f);		/* ntk byte */

	/* sort tracks numbers */
	c5 = 0x00;
	for (i = 0; i < len; i++) {
		if (i == 0) {
			plist[0] = c5;
			c5++;
			continue;
		}
		for (j = 0; j < i; j++) {
			status = 1;
			for (k = 0; k < 4; k++) {
				if (trknum[j][k] !=
					trknum[i][k]) {
					status = 0;
					break;
				}
			}
			if (status == 1) {
				plist[i] = plist[j];
				break;
			}
		}
		if (status == 0) {
			plist[i] = c5;
			c5++;
		}
		status = 1;
	}
	/* c5 is the max pattern number */

	/* create real list of tracks numbers for really existing patterns */
	c1 = 0;
	for (i = 0; i < len; i++) {
		if (i == 0) {
			real_tnum[c1][0] = trknum[i][0];
			real_tnum[c1][1] = trknum[i][1];
			real_tnum[c1][2] = trknum[i][2];
			real_tnum[c1][3] = trknum[i][3];
			c1++;
			continue;
		}
		for (j = 0; j < i; j++) {
			status = 1;
			if (plist[i] == plist[j]) {
				status = 0;
				break;
			}
		}
		if (status == 0)
			continue;
		real_tnum[c1][0] = trknum[i][0];
		real_tnum[c1][1] = trknum[i][1];
		real_tnum[c1][2] = trknum[i][2];
		real_tnum[c1][3] = trknum[i][3];
		c1++;
		status = 1;
	}

	fwrite(plist, 128, 1, out);	/* write pattern list */
	write32b(out, PW_MOD_MAGIC);	/* write ID */

	/* pattern data */
	for (i = 0; i < c5; i++) {
		memset(tmp, 0, 1024);
		memset(tdata, 0, 192 * 4);

		for (k = 0; k < 4; k++) {
			hio_seek(in, 1536 + 192 * real_tnum[i][k], SEEK_SET);
			hio_read(tdata[k], 192, 1, in);
		}

		for (j = 0; j < 64; j++) {
			int x = j * 16;

			for (k = 0; k < 4; k++) {
				uint8 *t = &tdata[k][j * 3];

				memcpy(tmp + x + k * 4, ptk_table[t[0]], 2);
				if ((t[1] & 0x0f) == 0x0d)
					t[1] -= 0x03;
				memcpy(tmp + x + k * 4 + 2, &t[1], 2);
			}
		}

		fwrite(tmp, 1024, 1, out);
	}

	/* sample data */
	hio_seek(in, 1536 + (192 * (max_trknum + 1)), SEEK_SET);
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_ksm (uint8 *data, char *t, int s)
{
	int i, j;
	int max_trk;

	PW_REQUEST_DATA(s, 1536);

	if (data[0] != 'M' || data[1] != '.')
		return -1;

	/* test "a" */
	if (data[15] != 'a')
		return -1;

	/* test volumes */
	for (i = 0; i < 15; i++) {
		if (data[54 + i * 32] > 0x40)
			return -1;
	}


	/* test tracks data */
	/* first, get the highest track number .. */
	max_trk = 0;
	for (i = 0; i < 1024; i++) {
		int x = data[i + 512];
		if (x == 0xff)
			break;
		if (x > max_trk)
			max_trk = x;
	}

	if (i == 1024)
		return -1;

	if (max_trk == 0)
		return -1;

	PW_REQUEST_DATA(s, 1536 + max_trk * 192 + 63 * 3);

	/* real test on tracks data starts now */
	for (i = 0; i <= max_trk; i++) {
		uint8 *d = data + 1536 + i * 192;
		for (j = 0; j < 64; j++) {
			if (d[j * 3] > 0x24)
				return -1;
		}
	}

	pw_read_title(data + 2, t, 13);

	return 0;
}

const struct pw_format pw_ksm = {
	"Kefrens Sound Machine",
	test_ksm,
	depack_ksm
};

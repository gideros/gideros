/*
 * Wanton_Packer.c   Copyright (C) 1997 Asle / ReDoX
 *
 * Converts MODs converted with Wanton packer
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_wn(HIO_HANDLE *in, FILE * out)
{
	uint8 c1, c2, c3, c4;
	uint8 npat, max;
	uint8 tmp[1024];
	int ssize = 0;
	int i, j;

	/* read header */
	pw_move_data(out, in, 950);

	/* get whole sample size */
	for (i = 0; i < 31; i++) {
		hio_seek(in, 42 + i * 30, SEEK_SET);
		ssize += hio_read16b(in) * 2;
	}

	/* read size of pattern list */
	hio_seek(in, 950, SEEK_SET);
	write8(out, npat = hio_read8(in));

	hio_read(tmp, 129, 1, in);
	fwrite(tmp, 129, 1, out);

	/* write ptk's ID */
	write32b(out, PW_MOD_MAGIC);

	/* get highest pattern number */
	for (max = i = 0; i < 128; i++) {
		if (tmp[i + 1] > max)
			max = tmp[i + 1];
	}
	max++;

	/* pattern data */
	hio_seek(in, 1084, SEEK_SET);
	for (i = 0; i < max; i++) {
		for (j = 0; j < 256; j++) {
			c1 = hio_read8(in);
			c2 = hio_read8(in);
			c3 = hio_read8(in);
			c4 = hio_read8(in);

			write8(out, c1 * 0xf0 | ptk_table[c1 / 2][0]);
			write8(out, ptk_table[c1 / 2][1]);
			write8(out, ((c2 << 4) & 0xf0) | c3);
			write8(out, c4);
		}
	}

	/* sample data */
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_wn(uint8 *data, char *t, int s)
{
	PW_REQUEST_DATA(s, 1082);

	/* test 1 */
	if (data[1080] != 'W' || data[1081] !='N')
		return -1;

	/* test 2 */
	if (data[951] != 0x7f)
		return -1;

	/* test 3 */
	if (data[950] > 0x7f)
		return -1;

	pw_read_title(data, t, 20);

	return 0;
}

const struct pw_format pw_wn = {
	"Wanton Packer",
	test_wn,
	depack_wn
};

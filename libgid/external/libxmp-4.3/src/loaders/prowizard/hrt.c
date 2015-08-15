/*
 * Hornet_Packer.c Copyright (C) 1997 Asle / ReDoX
 *
 * Modified in 2009,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_hrt(HIO_HANDLE *in, FILE *out)
{
	uint8 buf[1024];
	uint8 c1, c2, c3, c4;
	int len, npat;
	int ssize = 0;
	int i, j;

	memset(buf, 0, 950);

	hio_read(buf, 950, 1, in);			/* read header */
	for (i = 0; i < 31; i++)		/* erase addresses */
		*(uint32 *)(buf + 38 + 30 * i) = 0;
	fwrite(buf, 950, 1, out);		/* write header */

	for (i = 0; i < 31; i++)		/* samples size */
		ssize += readmem16b(buf + 42 + 30 * i) * 2;

	write8(out, len = hio_read8(in));		/* song length */
	write8(out, hio_read8(in));			/* nst byte */

	hio_read(buf, 1, 128, in);			/* pattern list */

	npat = 0;				/* number of patterns */
	for (i = 0; i < 128; i++) {
		if (buf[i] > npat)
			npat = buf[i];
	}
	npat++;

	write32b(out, PW_MOD_MAGIC);		/* write ptk ID */

	/* pattern data */
	hio_seek(in, 1084, SEEK_SET);
	for (i = 0; i < npat; i++) {
		for (j = 0; j < 256; j++) {
			buf[0] = hio_read8(in);
			buf[1] = hio_read8(in);
			buf[2] = hio_read8(in);
			buf[3] = hio_read8(in);

			buf[0] /= 2;
			c1 = buf[0] & 0xf0;

			if (buf[1] == 0)
				c2 = 0;
			else {
				c1 |= ptk_table[buf[1] / 2][0];
				c2 = ptk_table[buf[1] / 2][1];
			}

			c3 = ((buf[0] << 4) & 0xf0) | buf[2];
			c4 = buf[3];

			write8(out, c1);
			write8(out, c2);
			write8(out, c3);
			write8(out, c4);
		}
	}

	/* sample data */
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_hrt(uint8 *data, char *t, int s)
{
	int i;

	PW_REQUEST_DATA(s, 1084);

	if (readmem32b(data + 1080) != MAGIC4('H','R','T','!'))
		return -1;

	for (i = 0; i < 31; i++) {
		uint8 *d = data + 20 + i * 30;

		/* test finetune */
		if (d[24] > 0x0f)
			return -1;

		/* test volume */
		if (d[25] > 0x40)
			return -1;
	}

	pw_read_title(data, t, 20);

	return 0;
}

const struct pw_format pw_hrt = {
	"Hornet Packer",
	test_hrt,
	depack_hrt
};

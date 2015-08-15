/* Extended Module Player
 * Copyright (C) 1996-2015 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

#include "stdio2.h"
#include <stdlib.h>
#include <string.h>
#include "depacker.h"
#include "xz.h"
#include "crc32.h"

#define BUFFER_SIZE 4096

static int test_xz(unsigned char *b)
{
	return b[0] == 0xfd && b[3] == 'X' && b[4] == 'Z' && b[5] == 0x00;
}

static int decrunch_xz(FILE *in, FILE *out)
{
	struct xz_buf b;
	struct xz_dec *state;
	unsigned char *membuf;
	int ret = 0;

	crc32_init_A();

	memset(&b, 0, sizeof(b));
	if ((membuf = malloc(2 * BUFFER_SIZE)) == NULL)
		return -1;

	b.in = membuf;
	b.out = membuf + BUFFER_SIZE;
	b.out_size = BUFFER_SIZE;

	/* Limit memory usage to 16M */
	state = xz_dec_init(XZ_DYNALLOC, 16 * 1024 * 1024);

	while (1) {
		enum xz_ret r;

		if (b.in_pos == b.in_size) {
			int rd = fread(membuf, 1, BUFFER_SIZE, in);
			if (rd < 0) {
				ret = -1;
				break;
			}
			b.in_size = rd;
			b.in_pos = 0;
		}

		r = xz_dec_run(state, &b);

		if (b.out_pos) {
			fwrite(b.out, 1, b.out_pos, out);
			b.out_pos = 0;
		}

		if (r == XZ_STREAM_END) {
			break;
		}

		if (r != XZ_OK && r != XZ_UNSUPPORTED_CHECK) {
			ret = -1;
			break;
		}
	}

	xz_dec_end(state);
	free(membuf);

	return ret;
}

struct depacker xz_depacker = {
	test_xz,
	decrunch_xz
};

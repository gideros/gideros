/*
 * Promizer_18a.c   Copyright (C) 1997 Asle / ReDoX
 *
 * Converts PM18a packed MODs back to PTK MODs
 * thanks to Gryzor and his ProWizard tool ! ... without it, this prog
 * would not exist !!!
 *
 * Modified in 2006,2007,2014 by Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include "prowiz.h"


static int depack_p18a(HIO_HANDLE *in, FILE *out)
{
	short pat_max;
	int tmp_ptr;
	int refmax;
	uint8 pnum[128];
	int paddr[128];
	short pptr[64][256];
	int num_pat;
	uint8 *reftab;
	uint8 pat[128][1024];
	int i, j, k, l;
	int size, ssize;
	int psize;
	int smp_ofs;
	uint8 fin[31];
	uint8 oldins[4];

	memset(pnum, 0, 128);
	memset(pptr, 0, 64 << 8);
	memset(pat, 0, 128 * 1024);
	memset(fin, 0, 31);
	memset(oldins, 0, 4);
	memset(paddr, 0, 128 * 4);

	pw_write_zero(out, 20);				/* title */

	/* bypass replaycode routine */
	hio_seek(in, 4464, SEEK_SET);

	ssize = 0;
	for (i = 0; i < 31; i++) {
		pw_write_zero(out, 22);			/* sample name */
		write16b(out, size = hio_read16b(in));
		ssize += size * 2;
		write8(out, fin[i] = hio_read8(in));	/* finetune table */
		write8(out, hio_read8(in));		/* volume */
		write16b(out, hio_read16b(in));	/* loop start */
		write16b(out, hio_read16b(in));	/* loop size */
	}

	write8(out, num_pat = hio_read16b(in) / 4);	/* pattern table length */
	write8(out, 0x7f);			/* NoiseTracker restart byte */

	for (i = 0; i < 128; i++)
		paddr[i] = hio_read32b(in);

	/* ordering of patterns addresses */

	tmp_ptr = 0;
	for (i = 0; i < num_pat; i++) {
		if (i == 0) {
			pnum[0] = 0;
			tmp_ptr++;
			continue;
		}
		for (j = 0; j < i; j++) {
			if (paddr[i] == paddr[j]) {
				pnum[i] = pnum[j];
				break;
			}
		}
		if (j == i)
			pnum[i] = tmp_ptr++;
	}

	pat_max = tmp_ptr - 1;

	fwrite(pnum, 128, 1, out);		/* pattern table */
	write32b(out, PW_MOD_MAGIC);		/* M.K. */


	/* a little pre-calc code ... no other way to deal with these unknown
	 * pattern data sizes ! :(
	 */
	hio_seek(in, 4460, SEEK_SET);
	psize = hio_read32b(in);
	hio_seek(in, 5226, SEEK_SET);	/* back to pattern data start */

	/* now, reading all pattern data to get the max value of note */
	refmax = 0;
	for (j = 0; j < psize; j += 2) {
		int x = hio_read16b(in);
		if (x > refmax)
			refmax = x;
	}

	/* read "reference table" */
	refmax += 1;			/* 1st value is 0 ! */
	i = refmax * 4;			/* each block is 4 bytes long */
	reftab = (uint8 *) malloc(i);
	hio_read(reftab, i, 1, in);
	hio_seek(in, 5226, SEEK_SET);	/* back to pattern data start */

	for (j = 0; j <= pat_max; j++) {
		int flag = 0;
		hio_seek(in, paddr[j] + 5226, 0);
		for (i = 0; i < 64; i++) {
			for (k = 0; k < 4; k++) {
				uint8 *p = &pat[j][i * 16 + k * 4];
				int x = hio_read16b(in) << 2;
				int fine, ins, per, fxt;

				memcpy(p, &reftab[x], 4);

				ins = ((p[2] >> 4) & 0x0f) | (p[0] & 0xf0);
				if (ins != 0) {
					oldins[k] = ins;
				}

				per = ((p[0] & 0x0f) << 8) | p[1];
				fxt = p[2] & 0x0f;
				fine = fin[oldins[k] - 1];

				if (per != 0 && oldins[k] > 0 && fine != 0) {
					for (l = 0; l < 36; l++) {
						if (tun_table[fine][l] == per) {
							p[0] &= 0xf0;
							p[0] |=
							    ptk_table[l + 1][0];
							p[1] =
							    ptk_table[l + 1][1];
							break;
						}
					}
				}

				if (fxt == 0x0d || fxt == 0x0b) {
					flag = 1;
				}
			}

			if (flag == 1) {
				break;
			}
		}
		fwrite(pat[j], 1024, 1, out);
	}

	/* printf ( "Highest value in pattern data : %d\n" , refmax ); */

	free(reftab);

	hio_seek(in, 4456, SEEK_SET);
	smp_ofs = hio_read32b(in);
	hio_seek(in, 4460 + smp_ofs, SEEK_SET);

	/* Now, it's sample data ... though, VERY quickly handled :) */
	pw_move_data(out, in, ssize);

	return 0;
}

static int test_p18a(uint8 * data, char *t, int s)
{
	uint8 magic[] = {
		0x60, 0x38, 0x60, 0x00, 0x00, 0xa0, 0x60, 0x00,
		0x01, 0x3e, 0x60, 0x00, 0x01, 0x0c, 0x48, 0xe7
	};

	/* test 1 */
	PW_REQUEST_DATA(s, 22);

	if (memcmp(data, magic, 16) != 0)
		return -1;

	/* test 2 */
	if (data[21] != 0xd2)
		return -1;

	/* test 3 */
	PW_REQUEST_DATA(s, 4460);
	/*j =*/ readmem32b(data + 4456);

#if 0
	if ((start + j + 4456) > in_size) {
		Test = BAD;
		return;
	}
#endif

	/* test 4 */
	PW_REQUEST_DATA(s, 4714);
	if (readmem16b(data + 4712) & 0x03)
		return -1;

	/* test 5 */
	if (data[36] != 0x11)
		return -1;

	/* test 6 */
	if (data[37] != 0x00)
		return -1;

	pw_read_title(NULL, t, 0);

	return 0;
}

const struct pw_format pw_p18a = {
	"Promizer 1.8a",
	test_p18a,
	depack_p18a
};

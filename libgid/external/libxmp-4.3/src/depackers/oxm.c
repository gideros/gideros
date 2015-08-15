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
#include "vorbis.h"
#include "common.h"
#include "depacker.h"

#define MAGIC_OGGS	0x4f676753


struct xm_instrument {
	uint32 len;
	uint8 buf[36];
};

int test_oxm(FILE *f)
{
	int i, j;
	int hlen, npat, len, plen;
	int nins, nsmp;
	uint32 ilen;
	int slen[256];
	uint8 buf[1024];

	fseek(f, 0, SEEK_SET);
	if (fread(buf, 1, 16, f) < 16)
		return -1;
	if (memcmp(buf, "Extended Module:", 16))
		return -1;
	
	fseek(f, 60, SEEK_SET);
	hlen = read32l(f);
	fseek(f, 6, SEEK_CUR);
	npat = read16l(f);
	nins = read16l(f);

	if (npat > 256 || nins > 128)
		return -1;
	
	fseek(f, 60 + hlen, SEEK_SET);

	for (i = 0; i < npat; i++) {
		len = read32l(f);
		fseek(f, 3, SEEK_CUR);
		plen = read16l(f);
		fseek(f, len - 9 + plen, SEEK_CUR);
	}

	for (i = 0; i < nins; i++) {
		ilen = read32l(f);
		if (ilen > 263)
			return -1;
		fseek(f, -4, SEEK_CUR);
		fread(buf, ilen, 1, f);		/* instrument header */
		nsmp = readmem16l(buf + 27);

		if (nsmp > 255)
			return -1;
		if (nsmp == 0)
			continue;

		/* Read instrument data */
		for (j = 0; j < nsmp; j++) {
			slen[j] = read32l(f);
			fseek(f, 36, SEEK_CUR);
		}

		/* Read samples */
		for (j = 0; j < nsmp; j++) {
			read32b(f);
			if (read32b(f) == MAGIC_OGGS)
				return 0;
			fseek(f, slen[j] - 8, SEEK_CUR);
		}
	}

	return -1;
}

static char *oggdec(FILE *f, int len, int res, int *newlen)
{
	int i, n, ch;
	/*int size;*/
	uint8 *data, *pcm;
	int16 *pcm16 = NULL;
	uint32 id;

	/* Sanity check */
	if (len < 4) {
		return NULL;
	}

	/*size =*/ read32l(f);
	id = read32b(f);
	fseek(f, -8, SEEK_CUR);

	if ((data = calloc(1, len)) == NULL)
		return NULL;

	read32b(f);
	fread(data, 1, len - 4, f);

	if (id != MAGIC_OGGS) {		/* copy input data if not Ogg file */
		*newlen = len;
		return (char *)data;
	}
	
	n = stb_vorbis_decode_memory(data, len, &ch, &pcm16);
	free(data);

	if (n <= 0) {
		free(pcm16);
		return NULL;
	}

	pcm = (uint8 *)pcm16;

	if (res == 8) {
		for (i = 0; i < n; i++) {
			pcm[i] = pcm16[i] >> 8;
		}
		pcm = realloc(pcm16, n);
		if (pcm == NULL) {
			free(pcm16);
			return NULL;
		}
		pcm16 = (int16 *)pcm;
	}

	/* Convert to delta */
	if (res == 8) {
		for (i = n - 1; i > 0; i--)
			pcm[i] -= pcm[i - 1];
		*newlen = n;
	} else {
		for (i = n - 1; i > 0; i--)
			pcm16[i] -= pcm16[i - 1];
		*newlen = n * 2;
	}


	return (char *)pcm;
}

static int decrunch_oxm(FILE *f, FILE *fo)
{
	int i, j, pos;
	int hlen, npat, len, plen;
	int nins, nsmp, size;
	uint32 ilen;
	uint8 buf[1024];
	struct xm_instrument xi[256];
	char *pcm[256];
	int newlen = 0;

	fseek(f, 60, SEEK_SET);
	hlen = read32l(f);
	fseek(f, 6, SEEK_CUR);
	npat = read16l(f);
	nins = read16l(f);
	
	fseek(f, 60 + hlen, SEEK_SET);

	for (i = 0; i < npat; i++) {
		len = read32l(f);
		fseek(f, 3, SEEK_CUR);
		plen = read16l(f);
		fseek(f, len - 9 + plen, SEEK_CUR);
	}

	pos = ftell(f);
	fseek(f, 0, SEEK_SET);
	move_data(fo, f, pos);			/* module header + patterns */

	for (i = 0; i < nins; i++) {
		ilen = read32l(f);
		if (ilen > 1024) {
			D_(D_CRIT "ilen=%d\n", ilen);
			return -1;
		}
		fseek(f, -4, SEEK_CUR);
		fread(buf, ilen, 1, f);		/* instrument header */
		buf[26] = 0;
		fwrite(buf, ilen, 1, fo);
		nsmp = readmem16l(buf + 27);
		size = readmem32l(buf + 29);

		if (nsmp == 0) {
			continue;
		}

		/* Sanity check */
		if (nsmp > 0x10 || (nsmp > 0 && size > 0x100)) {
			D_(D_CRIT "Sanity check: nsmp=%d size=%d", nsmp, size);
			return -1;
		}

		/* Read sample headers */
		for (j = 0; j < nsmp; j++) {
			xi[j].len = read32l(f);
			if (xi[j].len > MAX_SAMPLE_SIZE) {
				D_(D_CRIT "sample %d len = %d", j, xi[j].len);
				return -1;
			}
			fread(xi[j].buf, 1, 36, f);
		}

		/* Read samples */
		for (j = 0; j < nsmp; j++) {
			D_(D_INFO "sample=%d len=%d\n", j, xi[j].len);
			if (xi[j].len > 0) {
				int res = 8;
				if (xi[j].buf[10] & 0x10)
					res = 16;
				pcm[j] = oggdec(f, xi[j].len, res, &newlen);
				xi[j].len = newlen;

				if (pcm[j] == NULL) {
					return -1;
				}
			}
		}

		/* Write sample headers */
		for (j = 0; j < nsmp; j++) {
			write32l(fo, xi[j].len);
			fwrite(xi[j].buf, 1, 36, fo);
		}

		/* Write samples */
		for (j = 0; j < nsmp; j++) {
			if (xi[j].len > 0) {
				fwrite(pcm[j], 1, xi[j].len, fo);
				free(pcm[j]);
			}
		}
	}

	return 0;
}

struct depacker oxm_depacker = {
	NULL,
	decrunch_oxm
};

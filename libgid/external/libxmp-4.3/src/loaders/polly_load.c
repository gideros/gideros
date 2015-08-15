/* Extended Module Player
 * Copyright (C) 1996-2015 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

/*
 * Polly Tracker is a tracker for the Commodore 64 written by Aleksi Eeben.
 * See http://aleksieeben.blogspot.com/2007/05/polly-tracker.html for more
 * information.
 */

#include "loader.h"

static int polly_test(HIO_HANDLE *, char *, const int);
static int polly_load(struct module_data *, HIO_HANDLE *, const int);

const struct format_loader polly_loader = {
	"Polly Tracker",
	polly_test,
	polly_load
};

#define NUM_PAT 0x1f
#define PAT_SIZE (64 * 4)
#define ORD_OFS (NUM_PAT * PAT_SIZE)
#define SMP_OFS (NUM_PAT * PAT_SIZE + 256)


static void decode_rle(uint8 *out, HIO_HANDLE *f, int size)
{
	int i;

	for (i = 0; i < size; ) {
		int x = hio_read8(f);

		if (hio_eof(f))
			return;

		if (x == 0xae) {
			int n,v;
			switch (n = hio_read8(f)) {
			case 0x01:
				out[i++] = 0xae;
				break;
			default:
				v = hio_read8(f);
				while (n-- && i < size)
					out[i++] = v;
			}
		} else {
			out[i++] = x;
		}
	}
}

static int polly_test(HIO_HANDLE *f, char *t, const int start)
{
	int i;
	uint8 *buf;

	if (hio_read8(f) != 0xae)
		return -1;

	if ((buf = malloc(0x10000)) == NULL)
		return -1;

	decode_rle(buf, f, 0x10000);

	for (i = 0; i < 128; i++) {
		if (buf[ORD_OFS + i] != 0 && buf[ORD_OFS] < 0xe0) {
			free(buf);
			return -1;
		}
	}

	if (t) {
		memcpy(t, buf + ORD_OFS + 160, 16);
		t[16] = 0;
		for (i = 15; i >=0; i--) {
			if (t[i] == ' ') {
				t[i] = 0;
			} else {
				break;
			}
		}
	}

	free(buf);

	return 0;
}

static int polly_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
	struct xmp_module *mod = &m->mod;
	struct xmp_event *event;
	uint8 *buf;
	int i, j, k;

	LOAD_INIT();

	hio_read8(f);			/* skip 0xae */
	/*
	 * File is RLE-encoded, escape is 0xAE (Aleksi Eeben's initials).
	 * Actual 0xAE is encoded as 0xAE 0x01
	 */
	if ((buf = calloc(1, 0x10000)) == NULL)
		return -1;

	decode_rle(buf, f, 0x10000);

	for (i = 0; buf[ORD_OFS + i] != 0 && i < 128; i++)
		mod->xxo[i] = buf[ORD_OFS + i] - 0xe0;
	mod->len = i;

	memcpy(mod->name, buf + ORD_OFS + 160, 16);
	/* memcpy(m->author, buf + ORD_OFS + 176, 16); */
	set_type(m, "Polly Tracker");
	MODULE_INFO();

	mod->spd = 0x03;
	mod->bpm = 0x7d * buf[ORD_OFS + 193] / 0x88;
#if 0
	for (i = 0; i < 1024; i++) {
		if ((i % 16) == 0) printf("\n");
		printf("%02x ", buf[ORD_OFS + i]);
	}
#endif

	mod->pat = 0;
	for (i = 0; i < mod->len; i++) {
		if (mod->xxo[i] > mod->pat)
			mod->pat = mod->xxo[i];
	}
	mod->pat++;
	
	mod->chn = 4;
	mod->trk = mod->pat * mod->chn;

	if (pattern_init(mod) < 0) {
		free(buf);
		return -1;
	}

	D_(D_INFO "Stored patterns: %d", mod->pat);

	for (i = 0; i < mod->pat; i++) {
		if (pattern_tracks_alloc(mod, i, 64) < 0) {
			free(buf);
			return -1;
		}

		for (j = 0; j < 64; j++) {
			for (k = 0; k < 4; k++) {
				uint8 x = buf[i * PAT_SIZE + j * 4 + k];
				event = &EVENT(i, k, j);
				if (x == 0xf0) {
					event->fxt = FX_BREAK;
					event->fxp = 0;
					continue;
				}
				event->note = LSN(x);
				if (event->note)
					event->note += 48;
				event->ins = MSN(x);
			}
		}
	}

	mod->ins = mod->smp = 15;
	if (instrument_init(mod) < 0) {
		free(buf);
		return -1;
	}

	for (i = 0; i < 15; i++) {
		if (subinstrument_alloc(mod, i, 1) < 0) {
			free(buf);
			return -1;
		}
		mod->xxs[i].len = buf[ORD_OFS + 129 + i] < 0x10 ? 0 :
					256 * buf[ORD_OFS + 145 + i];
		mod->xxi[i].sub[0].fin = 0;
		mod->xxi[i].sub[0].vol = 0x40;
		mod->xxs[i].lps = 0;
		mod->xxs[i].lpe = 0;
		mod->xxs[i].flg = 0;
		mod->xxi[i].sub[0].pan = 0x80;
		mod->xxi[i].sub[0].sid = i;
		mod->xxi[i].nsm = !!(mod->xxs[i].len);
		mod->xxi[i].rls = 0xfff;

                D_(D_INFO "[%2X] %04x %04x %04x %c V%02x",
                       		i, mod->xxs[i].len, mod->xxs[i].lps,
                        	mod->xxs[i].lpe, ' ', mod->xxi[i].sub[0].vol);
	}

	/* Convert samples from 6 to 8 bits */
	for (i = SMP_OFS; i < 0x10000; i++)
		buf[i] = buf[i] << 2;

	/* Read samples */
	D_(D_INFO "Loading samples: %d", mod->ins);

	for (i = 0; i < mod->ins; i++) {
		int ret;

		if (mod->xxs[i].len == 0)
			continue;
		ret = load_sample(m, NULL, SAMPLE_FLAG_NOLOAD | SAMPLE_FLAG_UNS,
				&mod->xxs[i], (char*)buf + ORD_OFS + 256 +
					256 * (buf[ORD_OFS + 129 + i] - 0x10));

		if (ret < 0) {
			free(buf);
			return -1;
		}
	}

	free(buf);

	/* make it mono */
	for (i = 0; i < mod->chn; i++)
		mod->xxc[i].pan = 0x80;

	m->quirk |= QUIRK_MODRNG;

	return 0;
}

/* Extended Module Player
 * Copyright (C) 1996-2015 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * From http://www.tscc.de/ucm24/tcb2pro.html:
 * There are two different TCB-Tracker module formats. Older format and
 * newer format. They have different headers "AN COOL." and "AN COOL!".
 *
 * We only support the old format --claudio
 */

#include "loader.h"


static int tcb_test(HIO_HANDLE *, char *, const int);
static int tcb_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader tcb_loader = {
	"TCB Tracker",
	tcb_test,
	tcb_load
};

static int tcb_test(HIO_HANDLE *f, char *t, const int start)
{
	uint8 buffer[10];

	if (hio_read(buffer, 1, 8, f) < 8)
		return -1;
	if (memcmp(buffer, "AN COOL.", 8) && memcmp(buffer, "AN COOL!", 8))
		return -1;

	read_title(f, t, 0);

	return 0;
}

static int tcb_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
	struct xmp_module *mod = &m->mod;
	struct xmp_event *event;
	int i, j, k;
	uint8 buffer[10];
	int base_offs, soffs[16];
	uint8 unk1[16], unk2[16], unk3[16];

	LOAD_INIT();

	hio_read(buffer, 8, 1, f);

	set_type(m, "TCB Tracker", buffer);

	hio_read16b(f);	/* ? */
	mod->pat = hio_read16b(f);
	mod->ins = 16;
	mod->smp = mod->ins;
	mod->chn = 4;
	mod->trk = mod->pat * mod->chn;

	m->quirk |= QUIRK_MODRNG;

	hio_read16b(f);	/* ? */

	for (i = 0; i < 128; i++)
		mod->xxo[i] = hio_read8(f);

	mod->len = hio_read8(f);
	hio_read8(f);	/* ? */
	hio_read16b(f);	/* ? */

	MODULE_INFO();

	if (instrument_init(mod) < 0)
		return -1;

	/* Read instrument names */
	for (i = 0; i < mod->ins; i++) {
		if (subinstrument_alloc(mod, i, 1) < 0)
			return -1;
		hio_read(buffer, 8, 1, f);
		instrument_name(mod, i, buffer, 8);
	}

	hio_read16b(f);	/* ? */
	for (i = 0; i < 5; i++)
		hio_read16b(f);
	for (i = 0; i < 5; i++)
		hio_read16b(f);
	for (i = 0; i < 5; i++)
		hio_read16b(f);

	if (pattern_init(mod) < 0)
		return -1;

	/* Read and convert patterns */
	D_(D_INFO "Stored patterns: %d ", mod->pat);

	for (i = 0; i < mod->pat; i++) {
		if (pattern_tracks_alloc(mod, i, 64) < 0)
			return -1;

		for (j = 0; j < mod->xxp[i]->rows; j++) {
			for (k = 0; k < mod->chn; k++) {
				int b;
				event = &EVENT (i, k, j);

				b = hio_read8(f);
				if (b) {
					event->note = 12 * (b >> 4);
					event->note += (b & 0xf) + 36;
				}
				b = hio_read8(f);
				event->ins = b >> 4;
				if (event->ins)
					event->ins += 1;
				if (b &= 0x0f) {
					switch (b) {
					case 0xd:
						event->fxt = FX_BREAK;
						event->fxp = 0;
						break;
					default:
						printf("---> %02x\n", b);
					}
				}
			}
		}
	}

	base_offs = hio_tell(f);
	hio_read32b(f);	/* remaining size */

	/* Read instrument data */

	for (i = 0; i < mod->ins; i++) {
		mod->xxi[i].sub[0].vol = hio_read8(f) / 2;
		mod->xxi[i].sub[0].pan = 0x80;
		unk1[i] = hio_read8(f);
		unk2[i] = hio_read8(f);
		unk3[i] = hio_read8(f);
	}


	for (i = 0; i < mod->ins; i++) {
		soffs[i] = hio_read32b(f);
		mod->xxs[i].len = hio_read32b(f);
	}

	hio_read32b(f);
	hio_read32b(f);
	hio_read32b(f);
	hio_read32b(f);

	for (i = 0; i < mod->ins; i++) {
		mod->xxi[i].nsm = !!(mod->xxs[i].len);
		mod->xxs[i].lps = 0;
		mod->xxs[i].lpe = 0;
		mod->xxs[i].flg = mod->xxs[i].lpe > 0 ? XMP_SAMPLE_LOOP : 0;
		mod->xxi[i].sub[0].fin = 0;
		mod->xxi[i].sub[0].pan = 0x80;
		mod->xxi[i].sub[0].sid = i;

		D_(D_INFO "[%2X] %-8.8s  %04x %04x %04x %c "
						"V%02x  %02x %02x %02x\n",
				i, mod->xxi[i].name,
				mod->xxs[i].len, mod->xxs[i].lps, mod->xxs[i].lpe,
				mod->xxs[i].flg & XMP_SAMPLE_LOOP ? 'L' : ' ',
				mod->xxi[i].sub[0].vol, unk1[i], unk2[i], unk3[i]);
	}

	/* Read samples */

	D_(D_INFO "Stored samples: %d", mod->smp);

	for (i = 0; i < mod->ins; i++) {
		hio_seek(f, start + base_offs + soffs[i], SEEK_SET);
		if (load_sample(m, f, SAMPLE_FLAG_UNS, &mod->xxs[i], NULL) < 0)
			return -1;
	}

	return 0;
}

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

#include "loader.h"
#include "period.h"


static int gtk_test(HIO_HANDLE *, char *, const int);
static int gtk_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader gtk_loader = {
	"Graoumf Tracker",
	gtk_test,
	gtk_load
};

static int gtk_test(HIO_HANDLE * f, char *t, const int start)
{
	char buf[4];

	if (hio_read(buf, 1, 4, f) < 4)
		return -1;

	if (memcmp(buf, "GTK", 3) || buf[3] > 4)
		return -1;

	read_title(f, t, 32);

	return 0;
}

static void translate_effects(struct xmp_event *event)
{
	/* Ignore extended effects */
	if (event->fxt == 0x0e || event->fxt == 0x0c) {
		event->fxt = 0;
		event->fxp = 0;
	}

	/* handle high-numbered effects */
	if (event->fxt >= 0x40 && event->fxt <= 0x4f) {
		event->fxp = (event->fxt << 4) | (event->fxp >> 4);
		event->fxt = FX_SETPAN;
	} else if (event->fxt >= 0x80 && event->fxt <= 0x8f) {
		event->fxt = FX_MULTI_RETRIG;
	} else if (event->fxt >= 0x08) {
		switch (event->fxt) {
		case 0x08:	/* detune */
			event->fxt = event->fxp = 0;
			break;
		case 0x09:	/* delay */
			event->fxt = FX_EXTENDED;
			event->fxp = 0xd0 | (event->fxp & 0x0f);
			break;
		case 0x0a:	/* cut / note release */
			event->fxt = FX_EXTENDED;
			event->fxp = 0xc0 | (event->fxp & 0x0f);
			break;
		case 0x0b:	/* position jump */
		case 0x0d:	/* break pattern */
		case 0x0f:	/* set global speed/tempo */
			/* same as ptk */
			break;
		case 0x0c:	/* set vibrato waveform */
			event->fxt = FX_EXTENDED;
			event->fxp = 0x40 | (event->fxp & 0x0f);
			break;
		case 0x0e:	/* set tremolo waveform */
			event->fxt = FX_EXTENDED;
			event->fxp = 0x70 | (event->fxp & 0x0f);
			break;
		case 0x10:	/* arpeggio */
			event->fxt = FX_ARPEGGIO;
			break;
		case 0x11:	/* fine portamento up */
			event->fxt = FX_F_PORTA_UP;
			break;
		case 0x12:	/* fine portamento down */
			event->fxt = FX_F_PORTA_DN;
			break;
		case 0x13:	/* roll + volume slide */
			event->fxt = FX_MULTI_RETRIG;
			break;
		case 0x16:	/* exp. volume slide up */
		case 0x14:	/* linear volume slide up */
			event->fxt = FX_VOLSLIDE_UP;
			break;
		case 0x17:	/* exp. volume slide down */
		case 0x15:	/* linear volume slide down */
			event->fxt = FX_VOLSLIDE_DN;
			break;
		case 0x20:	/* set volume */
			event->fxt = FX_VOLSET;
			break;
		case 0x21:	/* set volume to 0x100 */
			event->fxt = FX_VOLSET;
			event->fxp = 0xff;
			break;
		case 0xa4:	/* fine volume slide up */
			event->fxt = FX_F_VSLIDE;
			if (event->fxp > 0x0f)
				event->fxp = 0x0f;
			event->fxp <<= 4;
			break;
		case 0xa5:	/* fine volume slide down */
			event->fxt = FX_F_VSLIDE;
			if (event->fxp > 0x0f)
				event->fxp = 0x0f;
			break;
		case 0xa8:	/* set number of frames */
			event->fxt = FX_S3M_SPEED;
			break;
		default:
			event->fxt = event->fxp = 0;
		}
	}
}

static int gtk_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
	struct xmp_module *mod = &m->mod;
	struct xmp_event *event;
	int i, j, k;
	uint8 buffer[40];
	int rows, bits, c2spd, size;
	int ver, patmax;

	LOAD_INIT();

	hio_read(buffer, 4, 1, f);
	ver = buffer[3];
	hio_read(mod->name, 32, 1, f);
	set_type(m, "Graoumf Tracker GTK v%d", ver);
	hio_seek(f, 160, SEEK_CUR);	/* skip comments */

	mod->ins = hio_read16b(f);
	mod->smp = mod->ins;
	rows = hio_read16b(f);
	mod->chn = hio_read16b(f);
	mod->len = hio_read16b(f);
	mod->rst = hio_read16b(f);
	m->volbase = 0x100;

	MODULE_INFO();

	D_(D_INFO "Instruments    : %d ", mod->ins);

	if (instrument_init(mod) < 0)
		return -1;

	for (i = 0; i < mod->ins; i++) {
		if (subinstrument_alloc(mod, i, 1) < 0)
			return -1;

		hio_read(buffer, 28, 1, f);
		instrument_name(mod, i, buffer, 28);

		if (ver == 1) {
			hio_read32b(f);
			mod->xxs[i].len = hio_read32b(f);
			mod->xxs[i].lps = hio_read32b(f);
			size = hio_read32b(f);
			mod->xxs[i].lpe = mod->xxs[i].lps + size - 1;
			hio_read16b(f);
			hio_read16b(f);
			mod->xxi[i].sub[0].vol = 0xff;
			mod->xxi[i].sub[0].pan = 0x80;
			bits = 1;
			c2spd = 8363;
		} else {
			hio_seek(f, 14, SEEK_CUR);
			hio_read16b(f);		/* autobal */
			bits = hio_read16b(f);	/* 1 = 8 bits, 2 = 16 bits */
			c2spd = hio_read16b(f);
			c2spd_to_note(c2spd, &mod->xxi[i].sub[0].xpo, &mod->xxi[i].sub[0].fin);
			mod->xxs[i].len = hio_read32b(f);
			mod->xxs[i].lps = hio_read32b(f);
			size = hio_read32b(f);
			mod->xxs[i].lpe = mod->xxs[i].lps + size - 1;
			mod->xxi[i].sub[0].vol = hio_read16b(f);
			hio_read8(f);
			mod->xxi[i].sub[0].fin = hio_read8s(f);
		}

		if (mod->xxs[i].len > 0)
			mod->xxi[i].nsm = 1;

		mod->xxi[i].sub[0].sid = i;
		mod->xxs[i].flg = size > 2 ? XMP_SAMPLE_LOOP : 0;

		if (bits > 1) {
			mod->xxs[i].flg |= XMP_SAMPLE_16BIT;
			mod->xxs[i].len >>= 1;
			mod->xxs[i].lps >>= 1;
			mod->xxs[i].lpe >>= 1;
		}

		D_(D_INFO "[%2X] %-28.28s  %05x%c%05x %05x %c "
						"V%02x F%+03d %5d", i,
			 	mod->xxi[i].name,
				mod->xxs[i].len,
				bits > 1 ? '+' : ' ',
				mod->xxs[i].lps,
				size,
				mod->xxs[i].flg & XMP_SAMPLE_LOOP ? 'L' : ' ',
				mod->xxi[i].sub[0].vol, mod->xxi[i].sub[0].fin,
				c2spd);
	}

	for (i = 0; i < 256; i++)
		mod->xxo[i] = hio_read16b(f);

	for (patmax = i = 0; i < mod->len; i++) {
		if (mod->xxo[i] > patmax)
			patmax = mod->xxo[i];
	}

	mod->pat = patmax + 1;
	mod->trk = mod->pat * mod->chn;

	if (pattern_init(mod) < 0)
		return -1;

	/* Read and convert patterns */
	D_(D_INFO "Stored patterns: %d", mod->pat);

	for (i = 0; i < mod->pat; i++) {
		if (pattern_tracks_alloc(mod, i, rows) < 0)
			return -1;

		for (j = 0; j < mod->xxp[i]->rows; j++) {
			for (k = 0; k < mod->chn; k++) {
				event = &EVENT (i, k, j);

				event->note = hio_read8(f);
				if (event->note) {
					event->note += 13;
				}
				event->ins = hio_read8(f);

				event->fxt = hio_read8(f);
				event->fxp = hio_read8(f);
				if (ver >= 4) {
					event->vol = hio_read8(f);
				}

				translate_effects(event);

			}
		}
	}

	/* Read samples */
	D_(D_INFO "Stored samples: %d", mod->smp);

	for (i = 0; i < mod->ins; i++) {
		if (mod->xxs[i].len == 0)
			continue;

		if (load_sample(m, f, 0, &mod->xxs[i], NULL) < 0)
			return -1;
	}

	return 0;
}

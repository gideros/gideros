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
#include "synth.h"

/* Based on the HSC File Format Spec, by Simon Peter <dn.tlp@gmx.net>
 * 
 * "Although the format is most commonly known through the HSC-Tracker by
 *  Electronic Rats, it was originally developed by Hannes Seifert of NEO
 *  Software for use in their commercial game productions in the time of
 *  1991 - 1994. ECR just ripped his player and coded an editor around it."
 */


static int hsc_test (HIO_HANDLE *, char *, const int);
static int hsc_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader hsc_loader = {
    "HSC-Tracker",
    hsc_test,
    hsc_load
};

static int hsc_test(HIO_HANDLE *f, char *t, const int start)
{
    int p, i, r, c;
    uint8 buf[1200];

    hio_seek(f, 128 * 12, SEEK_CUR);

    if (hio_read(buf, 1, 51, f) != 51)
	return -1;

    for (p = i = 0; i < 51; i++) {
	if (buf[i] == 0xff)
	    break;
	if (buf[i] > p)
	    p = buf[i];
    }
    if (!i || !p || i > 50 || p > 50)		/* Test number of patterns */
	return -1;		

    for (i = 0; i < p; i++) {
	hio_read(buf, 1, 64 * 9 * 2, f);
	for (r = 0; r < 64; r++) {
	    for (c = 0; c < 9; c++) {
		uint8 n = buf[r * 9 * 2 + c * 2];
		uint8 m = buf[r * 9 * 2 + c * 2 + 1];
		if (m > 0x06 && m < 0x10 && n != 0x80)	/* Test effects 07..0f */
		    return -1;
		if (MSN(m) > 6 && MSN(m) < 10)	/* Test effects 7x .. 9x */
		    return -1;
	    }
	}
    }

    read_title(f, t, 0);

    return 0;
}

static int hsc_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
    struct xmp_module *mod = &m->mod;
    int pat, i, r, c;
    struct xmp_event *event;
    uint8 *x, *sid, e[2], buf[128 * 12];

    LOAD_INIT();

    hio_read(buf, 1, 128 * 12, f);

    x = buf;
    for (i = 0; i < 128; i++, x += 12) {
	if (x[9] & ~0x3 || x[10] & ~0x3)	/* Test waveform register */
	    break;
	if (x[8] & ~0xf)			/* Test feedback & algorithm */
	    break;
    }

    mod->ins = i;

    hio_seek(f, start + 0, SEEK_SET);

    mod->chn = 9;
    mod->bpm = 135;
    mod->spd = 6;
    mod->smp = mod->ins;

    m->quirk |= QUIRK_LINEAR;

    set_type(m, "HSC-Tracker");

    MODULE_INFO();

    /* Read instruments */
    if (instrument_init(mod) < 0)
	return -1;

    hio_read (buf, 1, 128 * 12, f);
    sid = buf;
    for (i = 0; i < mod->ins; i++, sid += 12) {
	if (subinstrument_alloc(mod, i, 1) < 0)
	    return -1;

	mod->xxi[i].nsm = 1;
	mod->xxi[i].sub[0].vol = 0x40;
	mod->xxi[i].sub[0].fin = (int8)sid[11] / 4;
	mod->xxi[i].sub[0].pan = 0x80;
	mod->xxi[i].sub[0].xpo = 0;
	mod->xxi[i].sub[0].sid = i;
	mod->xxi[i].rls = LSN(sid[7]) * 32;	/* carrier release */

	if (load_sample(m, f, SAMPLE_FLAG_ADLIB | SAMPLE_FLAG_HSC,
					&mod->xxs[i], (char *)sid) < 0)
		return -1;
    }

    /* Read orders */
    for (pat = i = 0; i < 51; i++) {
	hio_read (&mod->xxo[i], 1, 1, f);
	if (mod->xxo[i] & 0x80)
	    break;			/* FIXME: jump line */
	if (mod->xxo[i] > pat)
	    pat = mod->xxo[i];
    }
    hio_seek(f, 50 - i, SEEK_CUR);
    mod->len = i;
    mod->pat = pat + 1;
    mod->trk = mod->pat * mod->chn;

    D_(D_INFO "Module length: %d", mod->len);
    D_(D_INFO "Instruments: %d", mod->ins);
    D_(D_INFO "Stored patterns: %d", mod->pat);

    if (pattern_init(mod) < 0)
	return -1;

    /* Read and convert patterns */
    for (i = 0; i < mod->pat; i++) {
	int ins[9] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

	if (pattern_tracks_alloc(mod, i, 64) < 0)
	    return -1;

        for (r = 0; r < mod->xxp[i]->rows; r++) {
            for (c = 0; c < 9; c++) {
	        hio_read (e, 1, 2, f);
	        event = &EVENT (i, c, r);
		if (e[0] & 0x80) {
		    ins[c] = e[1] + 1;
		} else if (e[0] == 0x7f) {
		    event->note = XMP_KEY_OFF;
		} else if (e[0] > 0) {
		    event->note = e[0] + 25;
		    event->ins = ins[c];
		}

		event->fxt = 0;
		event->fxp = 0;

		if (e[1] == 0x01) {
		    event->fxt = 0x0d;
		    event->fxp = 0;
		}
	    }
	}
    }

    for (i = 0; i < mod->chn; i++) {
	mod->xxc[i].pan = 0x80;
	mod->xxc[i].flg = XMP_CHANNEL_SYNTH;
    }

    m->synth = &synth_adlib;

    return 0;
}

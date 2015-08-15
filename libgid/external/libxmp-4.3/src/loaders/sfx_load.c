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

/* Reverse engineered from the two SFX files in the Delitracker mods disk
 * and music from Future Wars, Twinworld and Operation Stealth. Effects
 * must be verified/implemented.
 */

/* From the ExoticRipper docs:
 * [SoundFX 2.0 is] simply the same as SoundFX 1.3, except that it
 * uses 31 samples [instead of 15].
 */

#include "period.h"
#include "loader.h"

#define MAGIC_SONG	MAGIC4('S','O','N','G')


static int sfx_test (HIO_HANDLE *, char *, const int);
static int sfx_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader sfx_loader = {
    "SoundFX v1.3/2.0",
    sfx_test,
    sfx_load
};

static int sfx_test(HIO_HANDLE *f, char *t, const int start)
{
    uint32 a, b;

    hio_seek(f, 4 * 15, SEEK_CUR);
    a = hio_read32b(f);
    hio_seek(f, 4 * 15, SEEK_CUR);
    b = hio_read32b(f);

    if (a != MAGIC_SONG && b != MAGIC_SONG)
	return -1;

    read_title(f, t, 0);

    return 0;
}


struct sfx_ins {
    uint8 name[22];		/* Instrument name */
    uint16 len;			/* Sample length in words */
    uint8 finetune;		/* Finetune */
    uint8 volume;		/* Volume (0-63) */
    uint16 loop_start;		/* Sample loop start in bytes */
    uint16 loop_length;		/* Sample loop length in words */
};

struct sfx_header {
    uint32 magic;		/* 'SONG' */
    uint16 delay;		/* Delay value (tempo), default is 0x38e5 */
    uint16 unknown[7];		/* ? */
};

struct sfx_header2 {
    uint8 len;			/* Song length */
    uint8 restart;		/* Restart pos (?) */
    uint8 order[128];		/* Order list */
};


static int sfx_13_20_load(struct module_data *m, HIO_HANDLE *f, const int nins, const int start)
{
    struct xmp_module *mod = &m->mod;
    int i, j;
    struct xmp_event *event;
    struct sfx_header sfx;
    struct sfx_header2 sfx2;
    uint8 ev[4];
    int ins_size[31];
    struct sfx_ins ins[31];	/* Instruments */

    LOAD_INIT();

    for (i = 0; i < nins; i++)
	ins_size[i] = hio_read32b(f);

    sfx.magic = hio_read32b(f);
    sfx.delay = hio_read16b(f);
    if (sfx.delay < 178)//min value for 10000bpm
	return -1;

    hio_read(&sfx.unknown, 14, 1, f);

    if (sfx.magic != MAGIC_SONG)
	return -1;

    mod->chn = 4;
    mod->ins = nins;
    mod->smp = mod->ins;
    mod->bpm = 14565 * 122 / sfx.delay;

    for (i = 0; i < mod->ins; i++) {
	hio_read(&ins[i].name, 22, 1, f);
	ins[i].len = hio_read16b(f);
	ins[i].finetune = hio_read8(f);
	ins[i].volume = hio_read8(f);
	ins[i].loop_start = hio_read16b(f);
	ins[i].loop_length = hio_read16b(f);
    }

    sfx2.len = hio_read8(f);
    sfx2.restart = hio_read8(f);
    if (hio_read(&sfx2.order, 1, 128, f) != 128)
        return -1;

    mod->len = sfx2.len;
    if (mod->len > 0x7f)
	return -1;

    memcpy (mod->xxo, sfx2.order, mod->len);
    for (mod->pat = i = 0; i < mod->len; i++)
	if (mod->xxo[i] > mod->pat)
	    mod->pat = mod->xxo[i];
    mod->pat++;

    mod->trk = mod->chn * mod->pat;

    if (mod->ins == 15) {
        set_type(m, "SoundFX 1.3");
    } else {
        set_type(m, "SoundFX 2.0");
    }

    MODULE_INFO();

    if (instrument_init(mod) < 0)
	return -1;

    for (i = 0; i < mod->ins; i++) {
	struct xmp_instrument *xxi;
	struct xmp_subinstrument *sub;
	struct xmp_sample *xxs;

	if (subinstrument_alloc(mod, i, 1) < 0)
	    return -1;

	xxi = &mod->xxi[i];
	xxs = &mod->xxs[i];
	sub = &xxi->sub[0];

	xxs->len = ins_size[i];
	xxs->lps = ins[i].loop_start;
	xxs->lpe = xxs->lps + 2 * ins[i].loop_length;
	xxs->flg = ins[i].loop_length > 1 ? XMP_SAMPLE_LOOP : 0;
	xxi->nsm = 1;
	sub->vol = ins[i].volume;
	sub->fin = (int8)(ins[i].finetune << 4);
	sub->pan = 0x80;
	sub->sid = i;

	instrument_name(mod, i, ins[i].name, 22);

	D_(D_INFO "[%2X] %-22.22s %04x %04x %04x %c  %02x %+d",
		i, xxi->name, xxs->len, xxs->lps, xxs->lpe,
		xxs->flg & XMP_SAMPLE_LOOP ? 'L' : ' ', sub->vol,
		sub->fin >> 4);
    }

    if (pattern_init(mod) < 0)
	return -1;

    D_(D_INFO "Stored patterns: %d", mod->pat);

    for (i = 0; i < mod->pat; i++) {
	if (pattern_tracks_alloc(mod, i, 64) < 0)
	    return -1;

	for (j = 0; j < 64 * mod->chn; j++) {
	    event = &EVENT(i, j % mod->chn, j / mod->chn);
	    hio_read(ev, 1, 4, f);

	    event->note = period_to_note((LSN (ev[0]) << 8) | ev[1]);
	    event->ins = (MSN (ev[0]) << 4) | MSN (ev[2]);
	    event->fxp = ev[3];

	    switch (LSN(ev[2])) {
	    case 0x1:			/* Arpeggio */
		event->fxt = FX_ARPEGGIO;
		break;
	    case 0x02:			/* Pitch bend */
		if (event->fxp >> 4) {
		    event->fxt = FX_PORTA_DN;
		    event->fxp >>= 4;
		} else if (event->fxp & 0x0f) {
		    event->fxt = FX_PORTA_UP;
		    event->fxp &= 0x0f;
		}
		break;
	    case 0x5:			/* Volume up */
		event->fxt = FX_VOLSLIDE_DN;
		break;
	    case 0x6:			/* Set volume (attenuation) */
		event->fxt = FX_VOLSET;
		event->fxp = 0x40 - ev[3];
		break;
	    case 0x3:			/* LED on */
	    case 0x4:			/* LED off */
	    case 0x7:			/* Set step up */
	    case 0x8:			/* Set step down */
	    default:
		event->fxt = event->fxp = 0;
		break;
	    }
	}
    }

    m->quirk |= QUIRK_MODRNG;

    /* Read samples */

    D_(D_INFO "Stored samples: %d", mod->smp);

    for (i = 0; i < mod->ins; i++) {
	if (mod->xxs[i].len <= 2)
	    continue;
	if (load_sample(m, f, 0, &mod->xxs[i], NULL) < 0)
	    return -1;
    }

    return 0;
}


static int sfx_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
    if (sfx_13_20_load(m, f, 15, start) < 0)
	return sfx_13_20_load(m, f, 31, start);
    return 0;
}

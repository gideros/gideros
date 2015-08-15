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

static int amd_test(HIO_HANDLE *, char *, const int);
static int amd_load(struct module_data *, HIO_HANDLE *, const int);

const struct format_loader amd_loader = {
	"Amusic Adlib Tracker",
	amd_test,
	amd_load
};

static int amd_test(HIO_HANDLE *f, char *t, const int start)
{
	char buf[9];

	hio_seek(f, start + 1062, SEEK_SET);
	if (hio_read(buf, 1, 9, f) < 9)
		return -1;

	if (memcmp(buf, "<o", 2) || memcmp(buf + 6, "RoR", 3))
		return -1;

	hio_seek(f, start + 0, SEEK_SET);
	read_title(f, t, 24);

	return 0;
}

struct amd_instrument {
	uint8 name[23];		/* Instrument name */
	uint8 reg[11];		/* Adlib registers */
};

struct amd_file_header {
	uint8 name[24];		/* ASCIIZ song name */
	uint8 author[24];	/* ASCIIZ author name */
	struct amd_instrument ins[26];	/* Instruments */
	uint8 len;		/* Song length */
	uint8 pat;		/* Index of last pattern */
	uint8 order[128];	/* Orders */
	uint8 magic[9];		/* 3c 6f ef 51 55 ee 52 6f 52 */
	uint8 version;		/* 0x10=normal module, 0x11=packed */
};


static void read_event(uint8 b, struct xmp_event *event, HIO_HANDLE *f)
{
	event->fxp = b;
	b = hio_read8(f);	/* Instrument + effect type */
	event->ins = MSN(b);

	switch (b = LSN(b)) {
	case 0:	/* Arpeggio */
	case 1:	/* Slide up */
	case 2:	/* Slide down */
		break;
	case 8:	/* Tone portamento */
		b = FX_TONEPORTA;
		break;
	case 4:	/* Set volume */
		b = FX_VOLSET;
		break;
	case 3:	/* Modulator/carrier intensity */
	case 9:	/* Tremolo/vibrato */
		event->fxp = b = 0;
		break;
	case 5:	/* Pattern jump */
		b = FX_JUMP;
		break;
	case 6:	/* Pattern break */
		b = FX_BREAK;
		break;
	case 7:	/* Speed */
		if (event->fxp > 31) {
			event->fxp = b = 0;
			break;
		}
		b = FX_SPEED;
		break;
	}
	event->fxt = b;

	b = hio_read8(f);	/* Note + octave + instrument */
	event->ins |= (b & 1) << 4;
	if ((event->note = MSN(b)))
		event->note += (2 + ((b & 0xe) >> 1)) * 12;
}

static int load_unpacked_patterns(struct module_data *m, HIO_HANDLE *f)
{
	struct xmp_module *mod = &m->mod;
	int i, j;
	int tmode = 1;
	uint8 b;
	struct xmp_event *event;

	mod->trk = mod->pat * 9;
	if (pattern_init(mod) < 0)
		return -1;

	for (i = 0; i < mod->pat; i++) {
		if (pattern_tracks_alloc(mod, i, 64) < 0)
			return -1;

		for (j = 0; j < (64 * mod->chn); j++) {
			event = &EVENT(i, j % mod->chn, j / mod->chn);
			b = hio_read8(f);	/* Effect parameter */
			read_event(b, event, f);
			if (event->fxp == FX_SPEED)
				tmode = 3;
			event->fxp *= tmode;
		}
	}

	return 0;
}

static int load_packed_patterns(struct module_data *m, HIO_HANDLE *f)
{
	struct xmp_module *mod = &m->mod;
	int i, j, r;
	int tmode = 1;
	int stored_tracks;
	struct xmp_event *event;
	uint8 b;
	uint16 w;

	D_(D_INFO "Stored patterns: %d", mod->pat);
	mod->xxp = calloc(sizeof(struct xmp_pattern *), mod->pat + 1);
	if (mod->xxp == NULL)
		return -1;

	for (i = 0; i < mod->pat; i++) {
		if (pattern_alloc(mod, i) < 0)
			return -1;

		for (j = 0; j < 9; j++) {
			uint16 w = hio_read16l(f);
			mod->xxp[i]->index[j] = w;
			if (w > mod->trk)
				mod->trk = w;
		}
		mod->xxp[i]->rows = 64;
	}
	mod->trk++;

	/* Sanity check */
	if (mod->trk > mod->pat * 9) {
		return -1;
	}

	stored_tracks = hio_read16l(f);
	
	/* Sanity check */
	if (stored_tracks <= 0) {
		return -1;
	}

	D_(D_INFO "Tracks: %d", mod->trk);
	D_(D_INFO "Stored tracks: %d", stored_tracks);

	mod->xxt = calloc(sizeof(struct xmp_track *), mod->trk);
	if (mod->xxt == NULL)
		return -1;

	for (i = 0; i < stored_tracks; i++) {
		w = hio_read16l(f);

		/* Sanity check */
		if (w >= mod->trk || mod->xxt[w] != NULL) {
			return -1;
		}

		if (track_alloc(mod, w, 64) < 0) {
			return -1;
		}

		for (r = 0; r < 64; r++) {
			event = &mod->xxt[w]->event[r];

			/* check event packing */
			b = hio_read8(f);	/* Effect parameter */
			if (b & 0x80) {
				r += (b & 0x7f) - 1;
				continue;
			}
			read_event(b, event, f);
			if (event->fxp == FX_SPEED)
				tmode = 3;
			event->fxp *= tmode;
		}
	}

	return 0;
}

static int amd_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
	struct xmp_module *mod = &m->mod;
	int i, j;
	struct amd_file_header afh;
	char regs[11];
	const int reg_xlat[] = { 0, 5, 1, 6, 2, 7, 3, 8, 4, 9, 10 };

	LOAD_INIT();

	hio_read(&afh.name, 24, 1, f);
	hio_read(&afh.author, 24, 1, f);
	for (i = 0; i < 26; i++) {
		hio_read(&afh.ins[i].name, 23, 1, f);
		hio_read(&afh.ins[i].reg, 11, 1, f);
	}
	afh.len = hio_read8(f);
	afh.pat = hio_read8(f);
	hio_read(&afh.order, 128, 1, f);
	hio_read(&afh.magic, 9, 1, f);
	afh.version = hio_read8(f);

	if (afh.version != 0x10 && afh.version != 0x11)
		return -1;

	mod->chn = 9;
	mod->bpm = 125;
	mod->spd = 6;
	mod->len = afh.len;
	mod->pat = afh.pat + 1;
	mod->ins = 26;
	mod->smp = mod->ins;
	memcpy(mod->xxo, afh.order, mod->len);

	set_type(m, "Amusic Adlib Tracker");
	strncpy(mod->name, (char *)afh.name, 24);

	MODULE_INFO();
	D_(D_INFO "Instruments: %d", mod->ins);

	if (instrument_init(mod) < 0)
		return -1;

	/* Load instruments */
	for (i = 0; i < mod->ins; i++) {
		if (subinstrument_alloc(mod, i, 1) < 0)
			return -1;

		instrument_name(mod, i, afh.ins[i].name, 23);

		mod->xxi[i].sub[0].vol = 0x40;
		mod->xxi[i].sub[0].pan = 0x80;
		mod->xxi[i].sub[0].sid = i;
		mod->xxi[i].nsm = 1;

		for (j = 0; j < 11; j++)
			regs[j] = afh.ins[i].reg[reg_xlat[j]];

		D_(D_INFO "\n[%2X] %-23.23s", i, mod->xxi[i].name);

		if (load_sample(m, f, SAMPLE_FLAG_ADLIB, &mod->xxs[i],regs) < 0)
			return -1;
	}

	if (afh.version == 0x10) {
		if (load_unpacked_patterns(m, f) < 0)
			return -1;
	} else {
		if (load_packed_patterns(m, f) < 0)
			return -1;
	}



	for (i = 0; i < mod->chn; i++) {
		mod->xxc[i].pan = 0x80;
		mod->xxc[i].flg = XMP_CHANNEL_SYNTH;
	}

	m->synth = &synth_adlib;

	return 0;
}

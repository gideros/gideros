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

#define STM_TYPE_SONG	0x01
#define STM_TYPE_MODULE	0x02

struct stm_instrument_header {
	uint8 name[12];		/* ASCIIZ instrument name */
	uint8 id;		/* Id=0 */
	uint8 idisk;		/* Instrument disk */
	uint16 rsvd1;		/* Reserved */
	uint16 length;		/* Sample length */
	uint16 loopbeg;		/* Loop begin */
	uint16 loopend;		/* Loop end */
	uint8 volume;		/* Playback volume */
	uint8 rsvd2;		/* Reserved */
	uint16 c2spd;		/* C4 speed */
	uint32 rsvd3;		/* Reserved */
	uint16 paralen;		/* Length in paragraphs */
};

struct stm_file_header {
	uint8 name[20];		/* ASCIIZ song name */
	uint8 magic[8];		/* '!Scream!' */
	uint8 rsvd1;		/* '\x1a' */
	uint8 type;		/* 1=song, 2=module */
	uint8 vermaj;		/* Major version number */
	uint8 vermin;		/* Minor version number */
	uint8 tempo;		/* Playback tempo */
	uint8 patterns;		/* Number of patterns */
	uint8 gvol;		/* Global volume */
	uint8 rsvd2[13];	/* Reserved */
	struct stm_instrument_header ins[31];
};

static int stm_test(HIO_HANDLE *, char *, const int);
static int stm_load(struct module_data *, HIO_HANDLE *, const int);

const struct format_loader stm_loader = {
	"Scream Tracker 2",
	stm_test,
	stm_load
};

static int stm_test(HIO_HANDLE * f, char *t, const int start)
{
	char buf[8];

	hio_seek(f, start + 20, SEEK_SET);
	if (hio_read(buf, 1, 8, f) < 8)
		return -1;
	if (memcmp(buf, "!Scream!", 8) && memcmp(buf, "BMOD2STM", 8))
		return -1;

	hio_read8(f);

	if (hio_read8(f) != STM_TYPE_MODULE)
		return -1;

	if (hio_read8(f) < 1)	/* We don't want STX files */
		return -1;

	hio_seek(f, start + 0, SEEK_SET);
	read_title(f, t, 20);

	return 0;
}

#define FX_NONE		0xff

/*
 * Skaven's note from http://www.futurecrew.com/skaven/oldies_music.html
 *
 * FYI for the tech-heads: In the old Scream Tracker 2 the Arpeggio command
 * (Jxx), if used in a single row with a 0x value, caused the note to skip
 * the specified amount of halftones upwards halfway through the row. I used
 * this in some songs to give the lead some character. However, when played
 * in ModPlug Tracker, this effect doesn't work the way it did back then.
 */

static const uint8 fx[] = {
	FX_NONE,
	FX_SPEED,	/* A - Set tempo to [INFO]. 60 normal. */
	FX_JUMP,	/* B - Break pattern and jmp to order [INFO] */
	FX_BREAK,	/* C - Break pattern */
	FX_VOLSLIDE,	/* D - Slide volume; Hi-nibble=up, Lo-nibble=down */
	FX_PORTA_DN,	/* E - Slide down at speed [INFO] */
	FX_PORTA_UP,	/* F - Slide up at speed [INFO] */
	FX_TONEPORTA,	/* G - Slide to the note specified at speed [INFO] */
	FX_VIBRATO,	/* H - Vibrato; Hi-nibble, speed. Lo-nibble, size */
	FX_TREMOR,	/* I - Tremor; Hi-nibble, ontime. Lo-nibble, offtime */
	FX_ARPEGGIO	/* J - Arpeggio */
};

static int stm_load(struct module_data *m, HIO_HANDLE * f, const int start)
{
	struct xmp_module *mod = &m->mod;
	int i, j;
	struct xmp_event *event;
	struct stm_file_header sfh;
	uint8 b;
	int bmod2stm = 0;

	LOAD_INIT();

	hio_read(&sfh.name, 20, 1, f);	/* ASCIIZ song name */
	hio_read(&sfh.magic, 8, 1, f);	/* '!Scream!' */
	sfh.rsvd1 = hio_read8(f);	/* '\x1a' */
	sfh.type = hio_read8(f);	/* 1=song, 2=module */
	sfh.vermaj = hio_read8(f);	/* Major version number */
	sfh.vermin = hio_read8(f);	/* Minor version number */
	sfh.tempo = hio_read8(f);	/* Playback tempo */
	sfh.patterns = hio_read8(f);	/* Number of patterns */
	sfh.gvol = hio_read8(f);	/* Global volume */
	hio_read(&sfh.rsvd2, 13, 1, f);	/* Reserved */

	for (i = 0; i < 31; i++) {
		hio_read(&sfh.ins[i].name, 12, 1, f);	/* Instrument name */
		sfh.ins[i].id = hio_read8(f);		/* Id=0 */
		sfh.ins[i].idisk = hio_read8(f);	/* Instrument disk */
		sfh.ins[i].rsvd1 = hio_read16l(f);	/* Reserved */
		sfh.ins[i].length = hio_read16l(f);	/* Sample length */
		sfh.ins[i].loopbeg = hio_read16l(f);	/* Loop begin */
		sfh.ins[i].loopend = hio_read16l(f);	/* Loop end */
		sfh.ins[i].volume = hio_read8(f);	/* Playback volume */
		sfh.ins[i].rsvd2 = hio_read8(f);	/* Reserved */
		sfh.ins[i].c2spd = hio_read16l(f);	/* C4 speed */
		sfh.ins[i].rsvd3 = hio_read32l(f);	/* Reserved */
		sfh.ins[i].paralen = hio_read16l(f);	/* Length in paragraphs */
	}

	if (!strncmp((char *)sfh.magic, "BMOD2STM", 8))
		bmod2stm = 1;

	mod->chn = 4;
	mod->pat = sfh.patterns;
	mod->trk = mod->pat * mod->chn;
	mod->spd = MSN(sfh.tempo);
	mod->ins = 31;
	mod->smp = mod->ins;
	m->c4rate = C4_NTSC_RATE;

	copy_adjust(mod->name, sfh.name, 20);

	if (bmod2stm) {
		snprintf(mod->type, XMP_NAME_SIZE, "BMOD2STM STM");
	} else {
		snprintf(mod->type, XMP_NAME_SIZE, "Scream Tracker %d.%02d STM",
			 sfh.vermaj, sfh.vermin);
	}

	MODULE_INFO();

	if (instrument_init(mod) < 0)
		return -1;

	/* Read and convert instruments and samples */
	for (i = 0; i < mod->ins; i++) {
		if (subinstrument_alloc(mod, i, 1) < 0)
			return -1;

		mod->xxs[i].len = sfh.ins[i].length;
		mod->xxs[i].lps = sfh.ins[i].loopbeg;
		mod->xxs[i].lpe = sfh.ins[i].loopend;
		if (mod->xxs[i].lpe == 0xffff)
			mod->xxs[i].lpe = 0;
		mod->xxs[i].flg = mod->xxs[i].lpe > 0 ? XMP_SAMPLE_LOOP : 0;
		mod->xxi[i].sub[0].vol = sfh.ins[i].volume;
		mod->xxi[i].sub[0].pan = 0x80;
		mod->xxi[i].sub[0].sid = i;

		if (mod->xxs[i].len > 0)
			mod->xxi[i].nsm = 1;

		instrument_name(mod, i, sfh.ins[i].name, 12);

		D_(D_INFO "[%2X] %-14.14s %04x %04x %04x %c V%02x %5d", i,
		   mod->xxi[i].name, mod->xxs[i].len, mod->xxs[i].lps,
		   mod->xxs[i].lpe,
		   mod->xxs[i].flg & XMP_SAMPLE_LOOP ? 'L' : ' ',
		   mod->xxi[i].sub[0].vol, sfh.ins[i].c2spd);

		sfh.ins[i].c2spd = 8363 * sfh.ins[i].c2spd / 8448;
		c2spd_to_note(sfh.ins[i].c2spd, &mod->xxi[i].sub[0].xpo,
			      &mod->xxi[i].sub[0].fin);
	}

	hio_read(mod->xxo, 1, 128, f);

	for (i = 0; i < 128; i++)
		if (mod->xxo[i] >= mod->pat)
			break;

	mod->len = i;

	D_(D_INFO "Module length: %d", mod->len);

	if (pattern_init(mod) < 0)
		return -1;

	/* Read and convert patterns */
	D_(D_INFO "Stored patterns: %d", mod->pat);

	for (i = 0; i < mod->pat; i++) {
		if (pattern_tracks_alloc(mod, i, 64) < 0)
			return -1;

		for (j = 0; j < 64 * mod->chn; j++) {
			event = &EVENT(i, j % mod->chn, j / mod->chn);
			b = hio_read8(f);
			memset(event, 0, sizeof(struct xmp_event));
			switch (b) {
			case 251:
			case 252:
			case 253:
				break;
			case 255:
			default:
				event->note = b == 255 ? 0 :
					1 + LSN(b) + 12 * (3 + MSN(b));
				b = hio_read8(f);
				event->vol = b & 0x07;
				event->ins = (b & 0xf8) >> 3;
				b = hio_read8(f);
				event->vol += (b & 0xf0) >> 1;
				if (event->vol > 0x40)
					event->vol = 0;
				else
					event->vol++;
				event->fxt = fx[LSN(b)];
				event->fxp = hio_read8(f);
				switch (event->fxt) {
				case FX_SPEED:
					event->fxp = MSN(event->fxp);
					break;
				case FX_NONE:
					event->fxp = event->fxt = 0;
					break;
				}
			}
		}
	}

	/* Read samples */
	D_(D_INFO "Stored samples: %d", mod->smp);

	for (i = 0; i < mod->ins; i++) {
		if (mod->xxs[i].len > 1) {
			if (load_sample(m, f, 0, &mod->xxs[i], NULL) < 0)
				return -1;
		} else {
			mod->xxi[i].nsm = 0;
		}
	}

	m->quirk |= QUIRK_VSALL | QUIRKS_ST3;
	m->read_event_type = READ_EVENT_ST3;

	return 0;
}

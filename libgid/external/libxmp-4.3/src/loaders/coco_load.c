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

static int coco_test (HIO_HANDLE *, char *, const int);
static int coco_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader coco_loader = {
	"Coconizer",
	coco_test,
	coco_load
};

static int check_cr(uint8 *s, int n)
{
	while (n--) {
		if (*s++ == 0x0d)
			return 0;
	}

	return -1;
}

static int coco_test(HIO_HANDLE *f, char *t, const int start)
{
	uint8 x, buf[20];
	uint32 y;
	int n, i;

	x = hio_read8(f);

	/* check number of channels */
	if (x != 0x84 && x != 0x88)
		return -1;

	hio_read(buf, 1, 20, f);		/* read title */
	if (check_cr(buf, 20) != 0)
		return -1;

	n = hio_read8(f);			/* instruments */
	if (n > 100)
		return -1;

	hio_read8(f);			/* sequences */
	hio_read8(f);			/* patterns */

	y = hio_read32l(f);
	if (y < 64 || y > 0x00100000)	/* offset of sequence table */
		return -1;

	y = hio_read32l(f);			/* offset of patterns */
	if (y < 64 || y > 0x00100000)
		return -1;

	for (i = 0; i < n; i++) {
		int ofs = hio_read32l(f);
		int len = hio_read32l(f);
		int vol = hio_read32l(f);
		int lps = hio_read32l(f);
		int lsz = hio_read32l(f);

		if (ofs < 64 || ofs > 0x00100000)
			return -1;

		if (vol > 0xff)
			return -1;

		if (len > 0x00100000 || lps > 0x00100000 || lsz > 0x00100000)
			return -1;

		if (lps + lsz - 1 > len)
			return -1;

		hio_read(buf, 1, 11, f);
		if (check_cr(buf, 11) != 0)
			return -1;

		hio_read8(f);	/* unused */
	}

	hio_seek(f, start + 1, SEEK_SET);
	read_title(f, t, 20);

#if 0
	for (i = 0; i < 20; i++) {
		if (t[i] == 0x0d)
			t[i] = 0;
	}
#endif
	
	return 0;
}


static void fix_effect(struct xmp_event *e)
{
	switch (e->fxt) {
	case 0x00:			/* 00 xy Normal play or Arpeggio */
		e->fxt = FX_ARPEGGIO;
		/* x: first halfnote to add
		   y: second halftone to subtract */
		break;
	case 0x01:			/* 01 xx Slide Up */
	case 0x05:
		e->fxt = FX_PORTA_UP;
		break;
	case 0x02:			/* 02 xx Slide Down */
	case 0x06:
		e->fxt = FX_PORTA_DN;
		break;
	case 0x03:
		e->fxt = FX_VOLSLIDE_UP;	/* FIXME: it's fine */
		break;
	case 0x04:
		e->fxt = FX_VOLSLIDE_DN;	/* FIXME: it's fine */
		break;
	case 0x07:
		e->fxt = FX_SETPAN;
		break;
	case 0x08:			/* FIXME */
	case 0x09:
	case 0x0a:
	case 0x0b:
		e->fxt = e->fxp = 0;
		break;
	case 0x0c:
		e->fxt = FX_VOLSET;
		e->fxp = 0xff - e->fxp;
		break;
	case 0x0d:
		e->fxt = FX_BREAK;
		break;
	case 0x0e:
		e->fxt = FX_JUMP;
		break;
	case 0x0f:
		e->fxt = FX_SPEED;
		break;
	case 0x10:			/* unused */
		e->fxt = e->fxp = 0;
		break;
	case 0x11:
	case 0x12:			/* FIXME */
		e->fxt = e->fxp = 0;
		break;
	case 0x13:
		e->fxt = FX_VOLSLIDE_UP;
		break;
	case 0x14:
		e->fxt = FX_VOLSLIDE_DN;
		break;
	default:
		e->fxt = e->fxp = 0;
	}
}

static int coco_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
	struct xmp_module *mod = &m->mod;
	struct xmp_event *event;
	int i, j;
	int seq_ptr, pat_ptr, smp_ptr[100];

	LOAD_INIT();

	mod->chn = hio_read8(f) & 0x3f;
	read_title(f, mod->name, 20);

	for (i = 0; i < 20; i++) {
		if (mod->name[i] == 0x0d)
			mod->name[i] = 0;
	}

	set_type(m, "Coconizer");

	mod->ins = mod->smp = hio_read8(f);
	mod->len = hio_read8(f);
	mod->pat = hio_read8(f);
	mod->trk = mod->pat * mod->chn;

	seq_ptr = hio_read32l(f);
	pat_ptr = hio_read32l(f);

	MODULE_INFO();

	if (instrument_init(mod) < 0)
		return -1;

	m->vol_table = (int *)arch_vol_table;
	m->volbase = 0xff;

	for (i = 0; i < mod->ins; i++) {
		if (subinstrument_alloc(mod, i, 1) < 0)
			return -1;

		smp_ptr[i] = hio_read32l(f);
		mod->xxs[i].len = hio_read32l(f);
		mod->xxi[i].sub[0].vol = 0xff - hio_read32l(f);
		mod->xxi[i].sub[0].pan = 0x80;
		mod->xxs[i].lps = hio_read32l(f);
                mod->xxs[i].lpe = mod->xxs[i].lps + hio_read32l(f);
		if (mod->xxs[i].lpe)
			mod->xxs[i].lpe -= 1;
		mod->xxs[i].flg = mod->xxs[i].lps > 0 ?  XMP_SAMPLE_LOOP : 0;
		hio_read(mod->xxi[i].name, 1, 11, f);
		for (j = 0; j < 11; j++) {
			if (mod->xxi[i].name[j] == 0x0d)
				mod->xxi[i].name[j] = 0;
		}
		hio_read8(f);	/* unused */
		mod->xxi[i].sub[0].sid = i;

		if (mod->xxs[i].len > 0)
			mod->xxi[i].nsm = 1;

		D_(D_INFO "[%2X] %-10.10s  %05x %05x %05x %c V%02x",
				i, mod->xxi[i].name,
				mod->xxs[i].len, mod->xxs[i].lps, mod->xxs[i].lpe,
				mod->xxs[i].flg & XMP_SAMPLE_LOOP ? 'L' : ' ',
				mod->xxi[i].sub[0].vol);
	}

	/* Sequence */

	hio_seek(f, start + seq_ptr, SEEK_SET);
	for (i = 0; ; i++) {
		uint8 x = hio_read8(f);
		if (x == 0xff)
			break;
		mod->xxo[i] = x;
	}
	for (i++; i % 4; i++)	/* for alignment */
		hio_read8(f);


	/* Patterns */

	if (pattern_init(mod) < 0)
		return -1;

	D_(D_INFO "Stored patterns: %d", mod->pat);

	for (i = 0; i < mod->pat; i++) {
		if (pattern_tracks_alloc(mod, i, 64) < 0)
			return -1;

		for (j = 0; j < (64 * mod->chn); j++) {
			event = &EVENT (i, j % mod->chn, j / mod->chn);
			event->fxp = hio_read8(f);
			event->fxt = hio_read8(f);
			event->ins = hio_read8(f);
			event->note = hio_read8(f);
			if (event->note)
				event->note += 12;

			fix_effect(event);
		}
	}

	/* Read samples */

	D_(D_INFO "Stored samples : %d", mod->smp);

	for (i = 0; i < mod->ins; i++) {
		if (mod->xxi[i].nsm == 0)
			continue;

		hio_seek(f, start + smp_ptr[i], SEEK_SET);
		if (load_sample(m, f, SAMPLE_FLAG_VIDC, &mod->xxs[i], NULL) < 0)
			return -1;
	}

	for (i = 0; i < mod->chn; i++) {
		mod->xxc[i].pan = DEFPAN((((i + 3) / 2) % 2) * 0xff);
	}

	return 0;
}

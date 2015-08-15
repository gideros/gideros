/* Extended Module Player
 * Copyright (C) 1996-2015 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

#include "loader.h"
#include "period.h"
#include "spectrum.h"

/* ZX Spectrum Sound Tracker loader
 * Sound Tracker written by Jarek Burczynski (Bzyk), 1990
 */

static int stc_test(HIO_HANDLE *, char *, const int);
static int stc_load(struct module_data *, HIO_HANDLE *, const int);


const struct format_loader stc_loader = {
	"ZX Spectrum Sound Tracker (STC)",
	stc_test,
	stc_load
};


#define MAX_PAT 32

struct stc_ord {
	int pattern;
	int height;
};

struct stc_pat {
	int ch[3];
};

static int stc_test(HIO_HANDLE * f, char *t, const int start)
{
	int pos_ptr, orn_ptr, pat_ptr;
	int i, len, max_pat;

	hio_seek(f, start, SEEK_SET);

	if (hio_read8(f) > 0x20)			/* Check tempo */
		return -1;

	pos_ptr = hio_read16l(f);			/* Positions pointer */
	orn_ptr = hio_read16l(f);			/* Ornaments pointer */
	pat_ptr = hio_read16l(f);			/* Patterns pointer */

	if (pos_ptr < 138 || orn_ptr < 138 || pat_ptr < 138)
		return -1;

	hio_seek(f, start + pos_ptr, SEEK_SET);
	len = hio_read8(f) + 1;

	for (max_pat = i = 0; i < len; i++) {
		int pat = hio_read8(f);
		if (pat > MAX_PAT)		/* Check orders */
			return -1;
		if (pat > max_pat)
			max_pat = pat;
		hio_read8(f);
	}

	hio_seek(f, pat_ptr, SEEK_SET);

	for (i = 0; i < max_pat; i++) {
		int num = hio_read8(f);		/* Check track pointers */
		if (num != (i + 1))
			return -1;
		hio_read16l(f);
		hio_read16l(f);
		hio_read16l(f);
	}

	if (hio_read8(f) != 0xff)
		return -1;

	hio_seek(f, start + 7, SEEK_SET);
	read_title(f, t, 18);

	return 0;
}

static int stc_load(struct module_data *m, HIO_HANDLE * f, const int start)
{
	struct xmp_module *mod = &m->mod;
	struct xmp_event *event /*, *noise*/;
	int i, j;
	uint8 buf[100];
	int pos_ptr, orn_ptr, pat_ptr;
	struct stc_ord stc_ord[256];
	struct stc_pat stc_pat[MAX_PAT];
	int num, flag, orn;
	int *decoded;
	struct spectrum_extra *se;

	LOAD_INIT();

	mod->spd = hio_read8(f);		/* Speed */
	pos_ptr = hio_read16l(f);		/* Positions pointer */
	orn_ptr = hio_read16l(f);		/* Ornaments pointer */
	pat_ptr = hio_read16l(f);		/* Patterns pointer */

	hio_read(buf, 18, 1, f);		/* Title */
	copy_adjust(mod->name, (uint8 *)buf, 18);
	set_type(m, "ZX Spectrum Sound Tracker");

	hio_read16l(f);			/* Size */

	/* Read orders */

	hio_seek(f, pos_ptr, SEEK_SET);
	mod->len = hio_read8(f) + 1;

	for (num = i = 0; i < mod->len; i++) {
		stc_ord[i].pattern = hio_read8(f);
		stc_ord[i].height = hio_read8s(f);
		//printf("%d %d -- ", stc_ord[i].pattern, stc_ord[i].height);

		for (flag = j = 0; j < i; j++) {
			if (stc_ord[i].pattern == stc_ord[j].pattern &&
				stc_ord[i].height == stc_ord[j].height)
			{
				mod->xxo[i] = mod->xxo[j];
				flag = 1;
				break;
			}
		}
		if (!flag) {
			mod->xxo[i] = num++;
		}
		//printf("%d\n", mod->xxo[i]);
	}

	mod->chn = 3;
	mod->pat = num;
	mod->trk = mod->pat * mod->chn;
	mod->ins = 15;
	mod->smp = mod->ins;
	orn = (pat_ptr - orn_ptr) / 33;

	MODULE_INFO();

	/* Read patterns */

	if (pattern_init(mod) < 0)
		return -1;

	hio_seek(f, pat_ptr, SEEK_SET);
	decoded = calloc(mod->pat, sizeof(int));
	D_(D_INFO "Stored patterns: %d ", mod->pat);

	for (i = 0; i < MAX_PAT; i++) {
		if (hio_read8(f) == 0xff)
			break;
		stc_pat[i].ch[0] = hio_read16l(f);
		stc_pat[i].ch[1] = hio_read16l(f);
		stc_pat[i].ch[2] = hio_read16l(f);
	}

	for (i = 0; i < mod->len; i++) {		/* pattern */
		int src = stc_ord[i].pattern - 1;
		int dest = mod->xxo[i];
		int trans = stc_ord[i].height;

		if (decoded[dest])
			continue;

		//printf("%d/%d) Read pattern %d -> %d\n", i, mod->len, src, dest);

		if (pattern_tracks_alloc(mod, dest, 64) < 0)
			return -1;

		for (j = 0; j < 3; j++) {		/* row */
			int row = 0;
			int x;
			int rowinc = 0;
	
			hio_seek(f, stc_pat[src].ch[j], SEEK_SET);

			do {
				for (;;) {
					x = hio_read8(f);

					if (x == 0xff)
						break;
	
//printf("pat %d, channel %d, row %d, x = %02x\n", dest, j, row, x);
					event = &EVENT(dest, j, row);
				
					if (x <= 0x5f) {
						event->note = x + 18 + trans;
						row += 1 + rowinc;
						break;
					}

					if (x <= 0x6f) {
						event->ins = x - 0x5f - 1;
					} else if (x <= 0x7f) {
						/* ornament*/
						event->fxt = FX_SYNTH_0;
						event->fxp = x - 0x70;
					} else if (x == 0x80) {
						event->note = XMP_KEY_OFF;
						row += 1 + rowinc;
						break;
					} else if (x == 0x81) {
						/* ? */
						row += 1 + rowinc;
					} else if (x == 0x82) {
						/* disable ornament/envelope */
						event->fxt = FX_SYNTH_0;
						event->fxp = 0;
						event->f2t = FX_SYNTH_2;
						event->f2p = 0;
					} else if (x <= 0x8e) {
						/* envelope */
						event->fxt = FX_SYNTH_0 +
							x - 0x80;      /* R13 */
						event->fxp = hio_read8(f); /* R11 */
						event->f2t = FX_SYNTH_1;
						event->f2p = hio_read8(f); /* R12 */
					} else {
						rowinc = x - 0xa1;
					}
				}
			} while (x != 0xff);
		}

		decoded[dest] = 1;
	}

	free(decoded);

	/* Read instruments */

	if (instrument_init(mod) < 0)
		return -1;

	hio_seek(f, 27, SEEK_SET);

	D_(D_INFO "Instruments: %d", mod->ins);
	for (i = 0; i < mod->ins; i++) {
		struct spectrum_sample ss;

		memset(&ss, 0, sizeof (struct spectrum_sample));
		if (subinstrument_alloc(mod, i, 1) < 0)
			return -1;
		mod->xxi[i].nsm = 1;
		mod->xxi[i].sub[0].vol = 0x40;
		mod->xxi[i].sub[0].pan = 0x80;
		mod->xxi[i].sub[0].xpo = -1;
		mod->xxi[i].sub[0].sid = i;

		hio_read(buf, 1, 99, f);

		if (buf[97] == 0) {
			ss.loop = 32;
			ss.length = 33;
		} else {
			ss.loop = buf[97] - 1;
			if (ss.loop > 31)
				ss.loop = 31;
			ss.length = buf[97] + buf[98];
			if (ss.length > 32)
				ss.length = 32;
			if (ss.length == 0)
				ss.length = 1;
			if (ss.loop >= ss.length)
				ss.loop = ss.length - 1;

			if (ss.length < 32) {
				ss.length += 33 - (ss.loop + 1);
				ss.loop = 32;
			}
		}

		/* Read sample ticks */

		for (j = 0; j < 31; j++) {
			struct spectrum_stick *sst = &ss.stick[j];
			uint8 *chdata = &buf[1 + j * 3];
			
			memset(sst, 0, sizeof (struct spectrum_stick));

			if (~chdata[1] & 0x80) {
				sst->flags |= SPECTRUM_FLAG_MIXNOISE;
				sst->noise_env_inc = chdata[1] & 0x1f;

				if (sst->noise_env_inc & 0x10)
					sst->noise_env_inc |= 0xf0;
			}

			if (~chdata[1] & 0x40)
				sst->flags |= SPECTRUM_FLAG_MIXTONE;

			sst->vol = chdata[0] & 0x0f;

			sst->tone_inc = (((int)(chdata[0] & 0xf0)) << 4) |
						chdata[2];

			if (~chdata[1] & 0x20)
				sst->tone_inc = -sst->tone_inc;

			sst->flags |= SPECTRUM_FLAG_ENVELOPE;

			/*if (j != 0) {
				reportv(ctx, 1, "               ");
			}
			reportv(ctx, 1, "%02X %c%c%c %c%03x %x\n", j,
				sst->flags & SPECTRUM_FLAG_MIXTONE ? 'T' : 't',
				sst->flags & SPECTRUM_FLAG_MIXNOISE ? 'N' : 'n',
				sst->flags & SPECTRUM_FLAG_ENVELOPE ? 'E' : 'e',
				sst->tone_inc >= 0 ? '+' : '-',
				sst->tone_inc >= 0 ?
					sst->tone_inc : -sst->tone_inc,
				sst->vol);*/
			
		}

		if (load_sample(m, f, SAMPLE_FLAG_SPECTRUM, &mod->xxs[i],
							(char *)&ss) < 0) {
			return -1;
		}
	}
	
	/* Read ornaments */

	hio_seek(f, orn_ptr, SEEK_SET);
	m->extra = calloc(1, sizeof (struct spectrum_extra));
	se = m->extra;

	D_(D_INFO "Ornaments: %d", orn);
	for (i = 0; i < orn; i++) {
		int index;
		struct spectrum_ornament *so;

		index = hio_read8(f);		

		so = &se->ornament[index];
		so->length = 32;
		so->loop = 31;

		for (j = 0; j < 32; j++) {
			so->val[j] = hio_read8s(f);
		}
	}

	for (i = 0; i < 4; i++) {
		mod->xxc[i].pan = 0x80;
		mod->xxc[i].flg = XMP_CHANNEL_SYNTH;
	}
	
	m->synth = &synth_spectrum;

	return 0;
}

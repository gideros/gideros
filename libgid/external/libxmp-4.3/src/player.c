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
 * Sat, 18 Apr 1998 20:23:07 +0200  Frederic Bujon <lvdl@bigfoot.com>
 * Pan effect bug fixed: In Fastracker II the track panning effect erases
 * the instrument panning effect, and the same should happen in xmp.
 */

/*
 * Fri, 26 Jun 1998 13:29:25 -0400 (EDT)
 * Reported by Jared Spiegel <spieg@phair.csh.rit.edu>
 * when the volume envelope is not enabled (disabled) on a sample, and a
 * notoff is delivered to ft2 (via either a noteoff in the note column or
 * command Kxx [where xx is # of ticks into row to give a noteoff to the
 * sample]), ft2 will set the volume of playback of the sample to 00h.
 *
 * Claudio's fix: implementing effect K
 */

#include <stdlib.h>
#include <string.h>
#include "virtual.h"
#include "period.h"
#include "effects.h"
#include "player.h"
#include "mixer.h"
#ifndef LIBXMP_CORE_PLAYER
#include "synth.h"
#include "extras.h"
#endif

/* Values for multi-retrig */
static const struct retrig_control rval[] = {
	{   0,  1,  1 }, {  -1,  1,  1 }, {  -2,  1,  1 }, {  -4,  1,  1 },
	{  -8,  1,  1 }, { -16,  1,  1 }, {   0,  2,  3 }, {   0,  1,  2 },
	{   0,  1,  1 }, {   1,  1,  1 }, {   2,  1,  1 }, {   4,  1,  1 },
	{   8,  1,  1 }, {  16,  1,  1 }, {   0,  3,  2 }, {   0,  2,  1 },
	{   0,  0,  1 }		/* Note cut */
	
};


/*
 * "Anyway I think this is the most brilliant piece of crap we
 *  have managed to put up!"
 *			  -- Ice of FC about "Mental Surgery"
 */


#ifndef LIBXMP_CORE_PLAYER

/* From http://www.un4seen.com/forum/?topic=7554.0
 *
 * "Invert loop" effect replaces (!) sample data bytes within loop with their
 * bitwise complement (NOT). The parameter sets speed of altering the samples.
 * This effectively trashes the sample data. Because of that this effect was
 * supposed to be removed in the very next ProTracker versions, but it was
 * never removed.
 *
 * Prior to [Protracker 1.1A] this effect is called "Funk Repeat" and it moves
 * loop of the instrument (just the loop information - sample data is not
 * altered). The parameter is the speed of moving the loop.
 */

static const int invloop_table[] = {
	0, 5, 6, 7, 8, 10, 11, 13, 16, 19, 22, 26, 32, 43, 64, 128
};

static void update_invloop(struct module_data *m, struct channel_data *xc)
{
	struct xmp_sample *xxs = &m->mod.xxs[xc->smp];
	int len;

	xc->invloop.count += invloop_table[xc->invloop.speed];

	if ((xxs->flg & XMP_SAMPLE_LOOP) && xc->invloop.count >= 128) {
		xc->invloop.count = 0;
		len = xxs->lpe - xxs->lps;	

		if (++xc->invloop.pos > len) {
			xc->invloop.pos = 0;
		}

		if (~xxs->flg & XMP_SAMPLE_16BIT) {
			xxs->data[xxs->lps + xc->invloop.pos] ^= 0xff;
		}
	}
}

#endif

static int is_first_frame(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
#ifndef LIBXMP_CORE_DISABLE_IT
	struct module_data *m = &ctx->m;

	if (m->read_event_type == READ_EVENT_IT) {
		return p->frame % p->speed == 0;
	} else {
		return p->frame == 0;
	}
#else
	return p->frame == 0;
#endif
}

static void reset_channels(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct smix_data *smix = &ctx->smix;
	struct channel_data *xc;
	int i, j;

#ifndef LIBXMP_CORE_PLAYER
	m->synth->reset(ctx);

	for (i = 0; i < p->virt.virt_channels; i++) {
		void *extra;

		xc = &p->xc_data[i];
		extra = xc->extra;
		memset(xc, 0, sizeof (struct channel_data));
		xc->extra = extra;
		reset_channel_extras(ctx, xc);
		xc->ins = -1;
		xc->old_ins = 1;	/* raw value */
		xc->key = -1;
		xc->volume = m->volbase;
	}
#else
	for (i = 0; i < p->virt.virt_channels; i++) {
		xc = &p->xc_data[i];
		memset(xc, 0, sizeof (struct channel_data));
		xc->ins = -1;
		xc->old_ins = 1;	/* raw value */
		xc->key = -1;
		xc->volume = m->volbase;
	}
#endif

	for (i = 0; i < p->virt.num_tracks; i++) {
		xc = &p->xc_data[i];
		xc->filter.cutoff = 0xff;

		if (i >= mod->chn && i < mod->chn + smix->chn) {
			xc->mastervol = 0x40;
			xc->pan.val = 0x80;
		} else {
			xc->mastervol = mod->xxc[i].vol;
			xc->pan.val = mod->xxc[i].pan;
		}
		
		/* Amiga split channel */
		if (mod->xxc[i].flg & XMP_CHANNEL_SPLIT) {
			xc->split = ((mod->xxc[i].flg & 0x30) >> 4) + 1;
			/* Connect split channel pairs */
			for (j = 0; j < i; j++) {
				if (mod->xxc[j].flg & XMP_CHANNEL_SPLIT) {
					if (p->xc_data[j].split == xc->split) {
						p->xc_data[j].pair = i;
						xc->pair = j;
					}
				}
			}
		} else {
			xc->split = 0;
		}

		/* Surround channel */
		if (mod->xxc[i].flg & XMP_CHANNEL_SURROUND) {
			xc->pan.surround = 1;
		}
	}
}

static int check_delay(struct context_data *ctx, struct xmp_event *e, int chn)
{
	struct player_data *p = &ctx->p;
	struct channel_data *xc = &p->xc_data[chn];
	struct module_data *m = &ctx->m;

	/* Tempo affects delay and must be computed first */
	if ((e->fxt == FX_SPEED && e->fxp < 0x20) || e->fxt == FX_S3M_SPEED) {
		if (e->fxp) {
			p->speed = e->fxp;
		}
	}
	if ((e->f2t == FX_SPEED && e->f2p < 0x20) || e->f2t == FX_S3M_SPEED) {
		if (e->f2p) {
			p->speed = e->f2p;
		}
	}

	/* Delay event read */
	if (e->fxt == FX_EXTENDED && MSN(e->fxp) == EX_DELAY && LSN(e->fxp)) {
		xc->delay = LSN(e->fxp) + 1;
		goto do_delay;
	}
	if (e->f2t == FX_EXTENDED && MSN(e->f2p) == EX_DELAY && LSN(e->f2p)) {
		xc->delay = LSN(e->f2p) + 1;
		goto do_delay;
	}

	return 0;

    do_delay:
	memcpy(&xc->delayed_event, e, sizeof (struct xmp_event));

	if (e->ins) {
		xc->delayed_ins = e->ins;
	}

	if (HAS_QUIRK(QUIRK_RTDELAY)) {
		if (e->vol == 0 && e->f2t == 0 && e->ins == 0 && e->note == 0)
			xc->delayed_event.vol = xc->volume + 1;
		if (e->note == 0)
			xc->delayed_event.note = xc->key + 1;
		if (e->ins == 0)
			xc->delayed_event.ins = xc->old_ins;
	}

	return 1;
}

static inline void read_row(struct context_data *ctx, int pat, int row)
{
	int chn;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct player_data *p = &ctx->p;
	struct flow_control *f = &p->flow;
	struct xmp_event ev;

	for (chn = 0; chn < mod->chn; chn++) {
		const int num_rows = mod->xxt[TRACK_NUM(pat, chn)]->rows;
		if (row < num_rows) {
			memcpy(&ev, &EVENT(pat, chn, row), sizeof(ev));
		} else {
			memset(&ev, 0, sizeof(ev));
		}

		if (ev.note == XMP_KEY_OFF) {
			int env_on = 0;
			int ins = ev.ins - 1;

			if (IS_VALID_INSTRUMENT(ins) &&
			    (mod->xxi[ins].aei.flg & XMP_ENVELOPE_ON)) {
				env_on = 1;
			}

			if (ev.fxt == FX_EXTENDED && MSN(ev.fxp) == EX_DELAY) {
				if (ev.ins && (LSN(ev.fxp) || env_on)) {
					if (LSN(ev.fxp)) {
						ev.note = 0;
					}
					ev.fxp = ev.fxt = 0;
				}
			}
		}

		if (check_delay(ctx, &ev, chn) == 0) {
			if (!f->rowdelay_set || f->rowdelay > 0) {
				read_event(ctx, &ev, chn);
#ifndef LIBXMP_CORE_PLAYER
				med_hold_hack(ctx, pat, chn, row);
#endif
			}
		} else {
			/* Reset flags. See SlideDelay.it */
			p->xc_data[chn].flags = 0;
		}
	}
}

static inline int get_channel_vol(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	int root;

	/* channel is a root channel */
	if (chn < p->virt.num_tracks)
		return p->channel_vol[chn];

	/* channel is invalid */
	if (chn >= p->virt.virt_channels)
		return 0;

	/* root is invalid */
	root = virt_getroot(ctx, chn);
	if (root < 0)
		return 0;

	return p->channel_vol[root];
}


/*
 * Update channel data
 */

#define DOENV_RELEASE ((TEST_NOTE(NOTE_RELEASE) || act == VIRT_ACTION_OFF))

static void process_volume(struct context_data *ctx, int chn, int act)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct channel_data *xc = &p->xc_data[chn];
	struct xmp_instrument *instrument;
	int finalvol;
	uint16 vol_envelope;
	int gvol;
	int fade = 0;

	instrument = get_instrument(ctx, xc->ins);

	/* Keyoff and fadeout */

	/* Keyoff event in IT doesn't reset fadeout (see jeff93.it)
	 * In XM it depends on envelope (see graff-strange_land.xm vs
	 * Decibelter - Cosmic 'Wegian Mamas.xm)
	 */
	if (HAS_QUIRK(QUIRK_KEYOFF)) {
		/* If IT, only apply fadeout on note release if we don't
		 * have envelope, or if we have envelope loop
		 */
		if (TEST_NOTE(NOTE_RELEASE) || act == VIRT_ACTION_OFF) {
			if ((~instrument->aei.flg & XMP_ENVELOPE_ON) ||
			    (instrument->aei.flg & XMP_ENVELOPE_LOOP)) {
				fade = 1;
			}
		}
	} else {
		if (~instrument->aei.flg & XMP_ENVELOPE_ON) {
			if (TEST_NOTE(NOTE_RELEASE)) {
				xc->fadeout = 0;
			}
		}

		if (TEST_NOTE(NOTE_RELEASE) || act == VIRT_ACTION_OFF) {
			fade = 1;
		}
	}

	if (TEST_NOTE(NOTE_FADEOUT) || act == VIRT_ACTION_FADE) {
		fade = 1;
	}

	if (fade) {
		if (xc->fadeout > xc->ins_fade) {
			xc->fadeout -= xc->ins_fade;
		} else {
			xc->fadeout = 0;
			SET_NOTE(NOTE_END);
		}
	}

	switch (check_envelope_fade(&instrument->aei, xc->v_idx)) {
	case -1:
		SET_NOTE(NOTE_END);
		/* Don't reset channel, we may have a tone portamento later
		 * virt_resetchannel(ctx, chn);
		 */
		break;
	case 0:
		break;
	default:
		if (HAS_QUIRK(QUIRK_ENVFADE)) {
			SET_NOTE(NOTE_FADEOUT);
		}
	}

	if (!TEST_PER(VENV_PAUSE)) {
		xc->v_idx = update_envelope(&instrument->aei, xc->v_idx,
					DOENV_RELEASE, TEST(KEY_OFF),
					m->read_event_type == READ_EVENT_IT);
	}

	vol_envelope = get_envelope(&instrument->aei, xc->v_idx, 64);
	if (check_envelope_end(&instrument->aei, xc->v_idx)) {
		if (vol_envelope == 0)
			SET_NOTE(NOTE_END);
		SET_NOTE(NOTE_ENV_END);
	}

	/* If note ended in background channel, we can safely reset it */
	if (TEST_NOTE(NOTE_END) && chn >= p->virt.num_tracks) {
		virt_resetchannel(ctx, chn);
		return;
	}

#ifndef LIBXMP_CORE_PLAYER
	finalvol = extras_get_volume(ctx, xc);
#else
	finalvol = xc->volume;
#endif

	if (TEST(TREMOLO)) {
		finalvol += get_lfo(ctx, &xc->tremolo.lfo, 1 << 6, 0);
		if (!is_first_frame(ctx) || HAS_QUIRK(QUIRK_VIBALL)) {
			update_lfo(&xc->tremolo.lfo);
		}
	}

	CLAMP(finalvol, 0, m->volbase);

	finalvol = (finalvol * xc->fadeout) >> 6;	/* 16 bit output */

	if (HAS_QUIRK(QUIRK_ST3GVOL)) {
		gvol = 0x40;
	} else {
		gvol = p->gvol;
	}

	finalvol = (uint32)(vol_envelope * gvol * xc->mastervol / m->gvolbase *
				((int)finalvol * 0x40 / m->volbase)) >> 18;

	/* Apply channel volume */
	finalvol = finalvol * get_channel_vol(ctx, chn) / 100;

#ifndef LIBXMP_CORE_PLAYER
	/* Volume translation table (for PTM, ARCH, COCO) */
	if (m->vol_table) {
		finalvol = m->volbase == 0xff ?
		    m->vol_table[finalvol >> 2] << 2 :
		    m->vol_table[finalvol >> 4] << 4;
	}
#endif

	if (HAS_QUIRK(QUIRK_INSVOL)) {
		finalvol = (finalvol * instrument->vol * xc->gvl) >> 12;
	}

	if (xc->tremor.val) {
		if (xc->tremor.count == 0) {
			/* end of down cycle, set up counter for up  */
			xc->tremor.count = MSN(xc->tremor.val) | 0x80;
		} else if (xc->tremor.count == 0x80) {
			/* end of up cycle, set up counter for down */
			xc->tremor.count = LSN(xc->tremor.val);
		}

		xc->tremor.count--;

		if (~xc->tremor.count & 0x80) {
			finalvol = 0;
		}
	}

	if (chn < m->mod.chn)
		finalvol = finalvol * p->master_vol / 100;
	else
		finalvol = finalvol * p->smix_vol / 100;

	xc->info_finalvol = TEST_NOTE(NOTE_SAMPLE_END) ? 0 : finalvol;

	virt_setvol(ctx, chn, finalvol);

	/* Check Amiga split channel */
	if (xc->split) {
		virt_setvol(ctx, xc->pair, finalvol);
	}
}

static void process_frequency(struct context_data *ctx, int chn, int act)
{
#ifndef LIBXMP_CORE_DISABLE_IT
	struct mixer_data *s = &ctx->s;
#endif
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct channel_data *xc = &p->xc_data[chn];
	struct xmp_instrument *instrument;
	double period;
	int linear_bend;
	int frq_envelope;
	int arp, vibrato;
#ifndef LIBXMP_CORE_DISABLE_IT
	int cutoff, resonance;
#endif

	instrument = get_instrument(ctx, xc->ins);

	if (!TEST_PER(FENV_PAUSE)) {
		xc->f_idx = update_envelope(&instrument->fei, xc->f_idx,
					DOENV_RELEASE, TEST(KEY_OFF),
					m->read_event_type == READ_EVENT_IT);
	}
	frq_envelope = get_envelope(&instrument->fei, xc->f_idx, 0);

#ifndef LIBXMP_CORE_PLAYER
	/* Do note slide */

	if (TEST(NOTE_SLIDE)) {
		if (xc->noteslide.count == 0) {
			xc->note += xc->noteslide.slide;
			xc->period = note_to_period(xc->note, xc->finetune,
					HAS_QUIRK(QUIRK_LINEAR), xc->per_adj);
			xc->noteslide.count = xc->noteslide.speed;
		}
		xc->noteslide.count--;

		virt_setnote(ctx, chn, xc->note);
	}
#endif

	/* Instrument vibrato */
	vibrato = get_lfo(ctx, &xc->insvib.lfo, (1024 * (1 + xc->insvib.sweep)), 1);
	update_lfo(&xc->insvib.lfo);
	if (xc->insvib.sweep > 1) {
		xc->insvib.sweep -= 2;
	} else {
		xc->insvib.sweep = 0;
	}

	/* Vibrato */
	if (TEST(VIBRATO) || TEST_PER(VIBRATO)) {
		int shift = HAS_QUIRK(QUIRK_VIBHALF) ? 10 : 9;
		int vib = get_lfo(ctx, &xc->vibrato.lfo, 1 << shift, 1);

		if (HAS_QUIRK(QUIRK_VIBINV)) {
			vibrato -= vib;
		} else {
			vibrato += vib;
		}

		if (!is_first_frame(ctx) || HAS_QUIRK(QUIRK_VIBALL)) {
			update_lfo(&xc->vibrato.lfo);
		}
	}

	period = xc->period;
#ifndef LIBXMP_CORE_PLAYER
	period += extras_get_period(ctx, xc);
#endif

	linear_bend = period_to_bend(period + vibrato, xc->note,
			TEST(GLISSANDO), HAS_QUIRK(QUIRK_LINEAR), xc->per_adj);

	/* Envelope */

	if (xc->f_idx >= 0 && (~instrument->fei.flg & XMP_ENVELOPE_FLT)) {
		/* IT pitch envelopes are always linear, even in Amiga period
		 * mode. Each unit in the envelope scale is 1/25 semitone.
		 */
		linear_bend += frq_envelope << 7;
	}

	/* Arpeggio */

	arp = xc->arpeggio.val[xc->arpeggio.count];
	if (arp != 0) {
		linear_bend += (100 << 7) * arp;
	}

#ifndef LIBXMP_CORE_PLAYER
	linear_bend += extras_get_linear_bend(ctx, xc);
#endif

	/* For xmp_get_frame_info() */
	xc->info_pitchbend = linear_bend >> 7;
	xc->info_period = note_to_period_mix(xc->note, linear_bend);

	if (HAS_QUIRK(QUIRK_MODRNG)) {
		CLAMP(xc->info_period,
			note_to_period(83, xc->finetune, 0, 0) * 4096,
			note_to_period(48, xc->finetune, 0, 0) * 4096);
	} else if (xc->info_period <  (1 << 12)) {
		xc->info_period = (1 << 12);
	}

	virt_setbend(ctx, chn, linear_bend);

#ifndef LIBXMP_CORE_DISABLE_IT

	/* Process filter */

	if (!HAS_QUIRK(QUIRK_FILTER)) {
		return;
	}

	if (xc->f_idx >= 0 && (instrument->fei.flg & XMP_ENVELOPE_FLT)) {
		if (frq_envelope < 0xfe) {
			xc->filter.envelope = frq_envelope;
		}
		cutoff = xc->filter.cutoff * xc->filter.envelope >> 8;
	} else {
		cutoff = xc->filter.cutoff;
	}
	resonance = xc->filter.resonance;

	if (cutoff > 0xff) {
		cutoff = 0xff;
	} else if (cutoff < 0xff) {
		int a0, b0, b1;
		filter_setup(s->freq, cutoff, resonance, &a0, &b0, &b1);
		virt_seteffect(ctx, chn, DSP_EFFECT_FILTER_A0, a0);
		virt_seteffect(ctx, chn, DSP_EFFECT_FILTER_B0, b0);
		virt_seteffect(ctx, chn, DSP_EFFECT_FILTER_B1, b1);
		virt_seteffect(ctx, chn, DSP_EFFECT_RESONANCE, resonance);
	}

	/* Always set cutoff */
	virt_seteffect(ctx, chn, DSP_EFFECT_CUTOFF, cutoff);

#endif
}

static void process_pan(struct context_data *ctx, int chn, int act)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct mixer_data *s = &ctx->s;
	struct channel_data *xc = &p->xc_data[chn];
	struct xmp_instrument *instrument;
	int finalpan, panbrello = 0;
	int pan_envelope;

	instrument = get_instrument(ctx, xc->ins);

	if (!TEST_PER(PENV_PAUSE)) {
		xc->p_idx = update_envelope(&instrument->pei, xc->p_idx,
					DOENV_RELEASE, TEST(KEY_OFF),
					m->read_event_type == READ_EVENT_IT);
	}
	pan_envelope = get_envelope(&instrument->pei, xc->p_idx, 32);

	if (TEST(PANBRELLO)) {
		panbrello = get_lfo(ctx, &xc->panbrello.lfo, 512, 0);
		update_lfo(&xc->panbrello.lfo);
	}

	finalpan = xc->pan.val + panbrello + (pan_envelope - 32) *
				(128 - abs(xc->pan.val - 128)) / 32;

	if (s->format & XMP_FORMAT_MONO || xc->pan.surround) {
		finalpan = 0;
	} else {
		finalpan = (finalpan - 0x80) * s->mix / 100;
	}

	CLAMP(finalpan, -128, 127);

	xc->info_finalpan = finalpan + 0x80;

	if (xc->pan.surround) {
		virt_setpan(ctx, chn, PAN_SURROUND);
	} else {
		virt_setpan(ctx, chn, finalpan);
	}
}

static void update_volume(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct channel_data *xc = &p->xc_data[chn];

	/* Volume slides happen in all frames but the first, except when the
	 * "volume slide on all frames" flag is set.
	 */
	if (p->frame % p->speed != 0 || HAS_QUIRK(QUIRK_VSALL)) {
		if (TEST(GVOL_SLIDE)) {
			p->gvol += xc->gvol.slide;
		}

		if (TEST(VOL_SLIDE) || TEST_PER(VOL_SLIDE)) {
			xc->volume += xc->vol.slide;
		}

#ifndef LIBXMP_CORE_PLAYER
		if (TEST_PER(VOL_SLIDE)) {
			if (xc->vol.slide > 0 && xc->volume > m->volbase) {
				xc->volume = m->volbase;
				RESET_PER(VOL_SLIDE);
			}
			if (xc->vol.slide < 0 && xc->volume < 0) {
				xc->volume = 0;
				RESET_PER(VOL_SLIDE);
			}
		}
#endif

		if (TEST(VOL_SLIDE_2)) {
			xc->volume += xc->vol.slide2;
		}
		if (TEST(TRK_VSLIDE)) {
			xc->mastervol += xc->trackvol.slide;
		}
	}

	if (p->frame % p->speed == 0) {
		/* Process "fine" effects */
		if (TEST(FINE_VOLS))
			xc->volume += xc->vol.fslide;

#ifndef LIBXMP_CORE_DISABLE_IT
		if (TEST(FINE_VOLS_2))
			xc->volume += xc->vol.fslide2;
#endif

		if (TEST(TRK_FVSLIDE))
			xc->mastervol += xc->trackvol.fslide;

		if (TEST(GVOL_SLIDE))
			p->gvol += xc->gvol.fslide;
	}

	/* Clamp volumes */
	CLAMP(xc->volume, 0, m->volbase);
	CLAMP(p->gvol, 0, m->gvolbase);
	CLAMP(xc->mastervol, 0, m->volbase);

	if (xc->split) {
		p->xc_data[xc->pair].volume = xc->volume;
	}
}

static void update_frequency(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct channel_data *xc = &p->xc_data[chn];

	if (!is_first_frame(ctx) || HAS_QUIRK(QUIRK_PBALL)) {
		if (TEST(PITCHBEND) || TEST_PER(PITCHBEND)) {
			xc->period += xc->freq.slide;
		}

		/* Do tone portamento */
		if (TEST(TONEPORTA) || TEST_PER(TONEPORTA)) {
			int end = 0;
			if (xc->porta.dir > 0) {
				xc->period += xc->porta.slide;
				if (xc->period >= xc->porta.target)
					end = 1;
			} else {
				xc->period -= xc->porta.slide;
				if (xc->period <= xc->porta.target)
					end = 1;
			}

			if (end) {
				/* reached end */
				xc->period = xc->porta.target;
				xc->porta.dir = 0;
				RESET(TONEPORTA);
				RESET_PER(TONEPORTA);
			}
		} 
	}

	if (is_first_frame(ctx)) {
		if (TEST(FINE_BEND)) {
			xc->period += xc->freq.fslide;
		}

#ifndef LIBXMP_CORE_PLAYER
		if (TEST(FINE_NSLIDE)) {
			xc->note += xc->noteslide.fslide;
			xc->period = note_to_period(xc->note,
				xc->finetune, HAS_QUIRK(QUIRK_LINEAR),
				xc->per_adj);
		}
#endif
	}

	if (HAS_QUIRK(QUIRK_LINEAR)) {
		CLAMP(xc->period, MIN_PERIOD_L, MAX_PERIOD_L);
	} else if (HAS_QUIRK(QUIRK_MODRNG)) {
		CLAMP(xc->period, note_to_period(83, xc->finetune, 0, 0),
				  note_to_period(48, xc->finetune, 0, 0));
	}

	/* Check for invalid periods (from Toru Egashira's NSPmod)
	 * panic.s3m has negative periods
	 * ambio.it uses low (~8) period values
	 */
	if (xc->period < 1) {
		xc->volume = 0;
	}

	xc->arpeggio.count++;
	xc->arpeggio.count %= xc->arpeggio.size;
}

static void update_pan(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	struct channel_data *xc = &p->xc_data[chn];

	if (TEST(PAN_SLIDE)) {
		if (is_first_frame(ctx)) {
			xc->pan.val += xc->pan.fslide;
		} else {
			xc->pan.val += xc->pan.slide;
		}

		if (xc->pan.val < 0) {
			xc->pan.val = 0;
		} else if (xc->pan.val > 0xff) {
			xc->pan.val = 0xff;
		}
	}
}

static void play_channel(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	struct smix_data *smix = &ctx->smix;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct channel_data *xc = &p->xc_data[chn];
	int act;

	xc->info_finalvol = 0;

	/* IT tempo slide */
	if (!is_first_frame(ctx) && TEST(TEMPO_SLIDE)) {
		p->bpm += xc->tempo.slide;
		CLAMP(p->bpm, 0x20, 0xff);
	}

	/* Do delay */
	if (xc->delay > 0) {
		if (--xc->delay == 0) {
			read_event(ctx, &xc->delayed_event, chn);
		}
	}

	act = virt_cstat(ctx, chn);
	if (act == VIRT_INVALID) {
		/* We need this to keep processing global volume slides */
		update_volume(ctx, chn);
		return;
	}

	if (p->frame == 0 && act != VIRT_ACTIVE) {
		if (!IS_VALID_INSTRUMENT_OR_SFX(xc->ins) || act == VIRT_ACTION_CUT) {
			virt_resetchannel(ctx, chn);
			return;
		}
	}

	if (!IS_VALID_INSTRUMENT_OR_SFX(xc->ins))
		return;

#ifndef LIBXMP_CORE_PLAYER
	play_extras(ctx, xc, chn);
#endif

	/* Do cut/retrig */
	if (TEST(RETRIG)) {
		int cond = HAS_QUIRK(QUIRK_S3MRTG) ?
				--xc->retrig.count <= 0 :
				--xc->retrig.count == 0;

		if (cond) {
			if (xc->retrig.type < 0x10) {
				/* don't retrig on cut */
				virt_voicepos(ctx, chn, 0);
			} else {
				SET_NOTE(NOTE_END);
			}
			xc->volume += rval[xc->retrig.type].s;
			xc->volume *= rval[xc->retrig.type].m;
			xc->volume /= rval[xc->retrig.type].d;
                	xc->retrig.count = LSN(xc->retrig.val);
		}
        }

	/* Do keyoff */
	if (xc->keyoff) {
		if (--xc->keyoff == 0)
			SET_NOTE(NOTE_RELEASE);
	}

	process_volume(ctx, chn, act);
	process_frequency(ctx, chn, act);
	process_pan(ctx, chn, act);

	update_volume(ctx, chn);
	update_frequency(ctx, chn);
	update_pan(ctx, chn);

#ifndef LIBXMP_CORE_PLAYER
	if (HAS_QUIRK(QUIRK_PROTRACK) && xc->ins < mod->ins) {
		update_invloop(m, xc);
	}
#endif

	if (TEST_NOTE(NOTE_SUSEXIT)) {
		SET_NOTE(NOTE_RELEASE);
	}

	xc->info_position = virt_getvoicepos(ctx, chn);
}

/*
 * Event injection
 */

static void inject_event(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct smix_data *smix = &ctx->smix;
	int chn;
	
	for (chn = 0; chn < mod->chn + smix->chn; chn++) {
		struct xmp_event *e = &p->inject_event[chn];
		if (e->_flag > 0) {
			read_event(ctx, e, chn);
			e->_flag = 0;
		}
	}
}

/*
 * Sequencing
 */

static void next_order(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
	struct flow_control *f = &p->flow;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;

	do {
    		p->ord++;

		/* Restart module */
		if (p->ord >= mod->len || mod->xxo[p->ord] == 0xff) {
			if (mod->rst > mod->len ||
			    mod->xxo[mod->rst] >= mod->pat ||
			    p->ord < m->seq_data[p->sequence].entry_point) {
				p->ord = m->seq_data[p->sequence].entry_point;
			} else {
				if (get_sequence(ctx, mod->rst) == p->sequence) {
					p->ord = mod->rst;
				} else {
					p->ord = m->seq_data[p->sequence].entry_point;
				}
			}

			p->gvol = m->xxo_info[p->ord].gvl;
		}
	} while (mod->xxo[p->ord] >= mod->pat);

	p->current_time = m->xxo_info[p->ord].time;

	f->num_rows = mod->xxp[mod->xxo[p->ord]]->rows;
	if (f->jumpline >= f->num_rows)
		f->jumpline = 0;
	p->row = f->jumpline;
	f->jumpline = 0;

	p->pos = p->ord;
	p->frame = 0;

#ifndef LIBXMP_CORE_PLAYER
	/* Reset persistent effects at new pattern */
	if (HAS_QUIRK(QUIRK_PERPAT)) {
		int chn;
		for (chn = 0; chn < mod->chn; chn++) {
			p->xc_data[chn].per_flags = 0;
		}
	}
#endif
}

static void next_row(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
	struct flow_control *f = &p->flow;

	p->frame = 0;
	f->delay = 0;

	if (f->pbreak) {
		f->pbreak = 0;

		if (f->jump != -1) {
			p->ord = f->jump - 1;
			f->jump = -1;
		}

		next_order(ctx);
	} else {
		if (f->loop_chn) {
			p->row = f->loop[f->loop_chn - 1].start - 1;
			f->loop_chn = 0;
		}
	
		if (f->rowdelay == 0) {
			p->row++;
			f->rowdelay_set = 0;
		} else {
			f->rowdelay--;
		}
	
		/* check end of pattern */
		if (p->row >= f->num_rows) {
			next_order(ctx);
		}
	}
}

#ifndef LIBXMP_CORE_DISABLE_IT

/*
 * Set note action for virt_pastnote
 */
void player_set_release(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	struct channel_data *xc = &p->xc_data[chn];

	SET_NOTE(NOTE_RELEASE);
}

void player_set_fadeout(struct context_data *ctx, int chn)
{
	struct player_data *p = &ctx->p;
	struct channel_data *xc = &p->xc_data[chn];

	SET_NOTE(NOTE_FADEOUT);
}

#endif

static void update_from_ord_info(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct ord_data *oinfo = &m->xxo_info[p->ord];

	if (oinfo->speed)
		p->speed = oinfo->speed;
	p->bpm = oinfo->bpm;
	p->gvol = oinfo->gvl;
	p->current_time = oinfo->time;
	p->frame_time = m->time_factor * m->rrate / p->bpm;

#ifndef LIBXMP_CORE_PLAYER
	p->st26_speed = oinfo->st26_speed;
#endif
}

int xmp_start_player(xmp_context opaque, int rate, int format)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
#ifndef LIBXMP_CORE_PLAYER
	struct mixer_data *s = &ctx->s;
#endif
	struct smix_data *smix = &ctx->smix;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct flow_control *f = &p->flow;
	int i;
	int ret = 0;

	if (rate < XMP_MIN_SRATE || rate > XMP_MAX_SRATE)
		return -XMP_ERROR_INVALID;

	if (ctx->state < XMP_STATE_LOADED)
		return -XMP_ERROR_STATE;

	if (ctx->state > XMP_STATE_LOADED)
		xmp_end_player(opaque);

	if (mixer_on(ctx, rate, format, m->c4rate) < 0)
		return -XMP_ERROR_INTERNAL;

	p->master_vol = 100;
	p->smix_vol = 100;
	p->gvol = m->volbase;
	p->pos = p->ord = 0;
	p->frame = -1;
	p->row = 0;
	p->current_time = 0;
	p->loop_count = 0;
	p->sequence = 0;

	/* Unmute all channels and set default volume */
	for (i = 0; i < XMP_MAX_CHANNELS; i++) {
		p->channel_mute[i] = 0;
		p->channel_vol[i] = 100;
	}

	/* Skip invalid patterns at start (the seventh laboratory.it) */
	while (p->ord < mod->len && mod->xxo[p->ord] >= mod->pat) {
		p->ord++;
	}
	/* Check if all positions skipped */
	if (p->ord >= mod->len) {
		mod->len = 0;
	}

	if (mod->len == 0 || mod->chn == 0) {
		/* set variables to sane state */
		p->ord = p->scan[0].ord = 0;
		p->row = p->scan[0].row = 0;
		f->end_point = 0;
		f->num_rows = 0;
	} else {
		f->num_rows = mod->xxp[mod->xxo[p->ord]]->rows;
		f->end_point = p->scan[0].num;
	}

	update_from_ord_info(ctx);

	if (virt_on(ctx, mod->chn + smix->chn) != 0) {
		ret = -XMP_ERROR_INTERNAL;
		goto err;
	}

	f->delay = 0;
	f->jumpline = 0;
	f->jump = -1;
	f->pbreak = 0;
	f->rowdelay_set = 0;

	f->loop = calloc(p->virt.virt_channels, sizeof(struct pattern_loop));
	if (f->loop == NULL) {
		ret = -XMP_ERROR_SYSTEM;
		goto err;
	}

	p->xc_data = calloc(p->virt.virt_channels, sizeof(struct channel_data));
	if (p->xc_data == NULL) {
		ret = -XMP_ERROR_SYSTEM;
		goto err1;
	}

#ifndef LIBXMP_CORE_PLAYER
	for (i = 0; i < p->virt.virt_channels; i++) {
		struct channel_data *xc = &p->xc_data[i];
		if (new_channel_extras(ctx, xc) < 0)
			goto err2;
	}

	if (m->synth->init(ctx, s->freq) < 0) {
		ret = -XMP_ERROR_INTERNAL;
		goto err2;
	}

	m->synth->reset(ctx);
#endif
	reset_channels(ctx);

	ctx->state = XMP_STATE_PLAYING;

	return 0;

#ifndef LIBXMP_CORE_PLAYER
    err2:
	free(p->xc_data);
#endif
    err1:
	free(f->loop);
    err:
	return ret;
}

static void check_end_of_module(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
	struct flow_control *f = &p->flow;

	/* check end of module */
	if (p->ord == p->scan[p->sequence].ord &&
			p->row == p->scan[p->sequence].row) {
		if (f->end_point == 0) {
			p->loop_count++;
			f->end_point = p->scan[p->sequence].num;
			/* return -1; */
		}
		f->end_point--;
	}
}

int xmp_play_frame(xmp_context opaque)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct flow_control *f = &p->flow;
	int i;

	if (ctx->state < XMP_STATE_PLAYING)
		return -XMP_ERROR_STATE;

	if (mod->len <= 0 || mod->xxo[p->ord] == 0xff)
		return -XMP_END;

	/* check reposition */
	if (p->ord != p->pos) {
		int start = m->seq_data[p->sequence].entry_point;

		if (p->pos == -2) {		/* set by xmp_module_stop */
			return -XMP_END;	/* that's all folks */
		}

		if (p->pos == -1) {
			/* restart sequence */
			p->pos = start;
		}

		if (p->pos == start) {
			f->end_point = p->scan[p->sequence].num;
		}

		/* Check if lands after a loop point */
		if (p->pos > p->scan[p->sequence].ord) {
			f->end_point = 0;
		}

		f->jumpline = 0;
		f->jump = -1;

		p->ord = p->pos - 1;

		/* Stay inside our subsong */
		if (p->ord < start) {
			p->ord = start - 1;
		}

		next_order(ctx);

		update_from_ord_info(ctx);

		virt_reset(ctx);
		reset_channels(ctx);
	} else {
		p->frame++;
		if (p->frame >= (p->speed * (1 + f->delay))) {
			/* If break during pattern delay, next row is skipped.
			 * See corruption.mod order 1D (pattern 0D) last line:
			 * EE2 + D31 ignores D00 in order 1C line 31. Reported
			 * by The Welder <welder@majesty.net>, Jan 14 2012
			 */
			if (HAS_QUIRK(QUIRK_PROTRACK) && f->delay && f->pbreak)
			{
				next_row(ctx);
				check_end_of_module(ctx);
			}
			next_row(ctx);
		}
	}

	for (i = 0; i < mod->chn; i++) {
		struct channel_data *xc = &p->xc_data[i];
		RESET(KEY_OFF);
	}

	/* check new row */

	if (p->frame == 0) {			/* first frame in row */
		check_end_of_module(ctx);
		read_row(ctx, mod->xxo[p->ord], p->row);

#ifndef LIBXMP_CORE_PLAYER
		if (p->st26_speed) {
			if  (p->st26_speed & 0x10000) {
				p->speed = (p->st26_speed & 0xff00) >> 8;
			} else {
				p->speed = p->st26_speed & 0xff;
			}
			p->st26_speed ^= 0x10000;
		}
#endif
	}

	inject_event(ctx);

	/* play_frame */
	for (i = 0; i < p->virt.virt_channels; i++) {
		play_channel(ctx, i);
	}

	p->frame_time = m->time_factor * m->rrate / p->bpm;
	p->current_time += p->frame_time;

	mixer_softmixer(ctx);

	return 0;
}

int xmp_play_buffer(xmp_context opaque, void *out_buffer, int size, int loop)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	int ret = 0, filled = 0, copy_size;
	struct xmp_frame_info fi;

	/* Reset internal state
	 * Syncs buffer start with frame start */
	if (out_buffer == NULL) {
		p->loop_count = 0;
		p->buffer_data.consumed = 0;
		p->buffer_data.in_size = 0;
		return 0;
	}

	if (ctx->state < XMP_STATE_PLAYING)
		return -XMP_ERROR_STATE;

	/* Fill buffer */
	while (filled < size) {
		/* Check if buffer full */
		if (p->buffer_data.consumed == p->buffer_data.in_size) {
			ret = xmp_play_frame(opaque);
			xmp_get_frame_info(opaque, &fi);

			/* Check end of module */
			if (ret < 0 || (loop > 0 && fi.loop_count >= loop)) {
				/* Start of frame, return end of replay */
				if (filled == 0) {
					p->buffer_data.consumed = 0;
					p->buffer_data.in_size = 0;
					return -1;
				}

				/* Fill remaining of this buffer */
				memset((char *)out_buffer + filled, 0, size - filled);
				return 0;
			}

			p->buffer_data.consumed = 0;
			p->buffer_data.in_buffer = fi.buffer;
			p->buffer_data.in_size = fi.buffer_size;
		}

		/* Copy frame data to user buffer */
		copy_size = MIN(size - filled, p->buffer_data.in_size -
					p->buffer_data.consumed);
		memcpy((char *)out_buffer + filled, p->buffer_data.in_buffer +
					p->buffer_data.consumed, copy_size);
		p->buffer_data.consumed += copy_size;
		filled += copy_size;
	}

	return ret;
}
    
void xmp_end_player(xmp_context opaque)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct flow_control *f = &p->flow;
#ifndef LIBXMP_CORE_PLAYER
	struct module_data *m = &ctx->m;
	struct channel_data *xc;
	int i;
#endif

	if (ctx->state < XMP_STATE_PLAYING)
		return;

	ctx->state = XMP_STATE_LOADED;

#ifndef LIBXMP_CORE_PLAYER
	/* Free channel extras */
	for (i = 0; i < p->virt.virt_channels; i++) {
		xc = &p->xc_data[i];
		release_channel_extras(ctx, xc);
	}
#endif

	virt_off(ctx);
#ifndef LIBXMP_CORE_PLAYER
	m->synth->deinit(ctx);
#endif

	free(p->xc_data);
	free(f->loop);

	p->xc_data = NULL;
	f->loop = NULL;

	mixer_off(ctx);
}

void xmp_get_module_info(xmp_context opaque, struct xmp_module_info *info)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;

	if (ctx->state < XMP_STATE_LOADED)
		return;

	memcpy(info->md5, m->md5, 16);
	info->mod = mod;
	info->comment = m->comment;
	info->num_sequences = m->num_sequences;
	info->seq_data = m->seq_data;
	info->vol_base = m->volbase;
}

void xmp_get_frame_info(xmp_context opaque, struct xmp_frame_info *info)
{
	struct context_data *ctx = (struct context_data *)opaque;
	struct player_data *p = &ctx->p;
	struct mixer_data *s = &ctx->s;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	int chn, i;

	if (ctx->state < XMP_STATE_LOADED)
		return;

	chn = mod->chn;

	if (p->pos >= 0 && p->pos < mod->len) {
		info->pos = p->pos;
	} else {
		info->pos = 0;
	}

	info->pattern = mod->xxo[info->pos];

	if (info->pattern < mod->pat) {
		info->num_rows = mod->xxp[info->pattern]->rows;
	} else {
		info->num_rows = 0;
	}

	info->row = p->row;
	info->frame = p->frame;
	info->speed = p->speed;
	info->bpm = p->bpm;
	info->total_time = p->scan[p->sequence].time;
	info->frame_time = p->frame_time * 1000;
	info->time = p->current_time;
	info->buffer = s->buffer;

	info->total_size = XMP_MAX_FRAMESIZE;
	info->buffer_size = s->ticksize;
	if (~s->format & XMP_FORMAT_MONO) {
		info->buffer_size *= 2;
	}
	if (~s->format & XMP_FORMAT_8BIT) {
		info->buffer_size *= 2;
	}

	info->volume = p->gvol;
	info->loop_count = p->loop_count;
	info->virt_channels = p->virt.virt_channels;
	info->virt_used = p->virt.virt_used;

	info->sequence = p->sequence;

	if (p->xc_data != NULL) {
		for (i = 0; i < chn; i++) {
			struct channel_data *c = &p->xc_data[i];
			struct xmp_channel_info *ci = &info->channel_info[i];
			struct xmp_track *track;
			struct xmp_event *event;
			int trk;
	
			ci->note = c->key;
			ci->pitchbend = c->info_pitchbend;
			ci->period = c->info_period;
			ci->position = c->info_position;
			ci->instrument = c->ins;
			ci->sample = c->smp;
			ci->volume = c->info_finalvol >> 4;
			ci->pan = c->info_finalpan;
			ci->reserved = 0;
			memset(&ci->event, 0, sizeof(*event));
	
			if (info->pattern < mod->pat && info->row < info->num_rows) {
				trk = mod->xxp[info->pattern]->index[i];
				track = mod->xxt[trk];
				if (info->row < track->rows) {
					event = &track->event[info->row];
					memcpy(&ci->event, event, sizeof(*event));
				}
			}
		}
	}
}

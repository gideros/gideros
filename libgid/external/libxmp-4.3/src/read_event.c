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

#include <string.h>
#include <stdlib.h>
#include "common.h"
#include "player.h"
#include "effects.h"
#include "virtual.h"
#include "period.h"
#include "envelope.h"

#ifndef LIBXMP_CORE_PLAYER
#include "med_extras.h"
#endif


static inline void copy_channel(struct player_data *p, int to, int from)
{
	if (to > 0 && to != from) {
		memcpy(&p->xc_data[to], &p->xc_data[from],
					sizeof (struct channel_data));
	}
}

static inline int has_note_event(struct xmp_event *e)
{
	return (e->note && e->note <= XMP_MAX_KEYS);
}

static inline int is_valid_note(int note)
{
	return (note >= 0 && note < XMP_MAX_KEYS);
}

static struct xmp_subinstrument *get_subinstrument(struct context_data *ctx,
						   int ins, int key)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct xmp_instrument *instrument;

	if (IS_VALID_INSTRUMENT(ins)) {
		instrument = &mod->xxi[ins];
		if (is_valid_note(key)) {
			int mapped = instrument->map[key].ins;
			if (mapped != 0xff && mapped >= 0 && mapped < instrument->nsm)
			  	return &instrument->sub[mapped];
	  }
	}

	return NULL;
}

static void reset_envelopes(struct context_data *ctx, struct channel_data *xc)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;

	if (!IS_VALID_INSTRUMENT(xc->ins))
		return;

 	RESET_NOTE(NOTE_ENV_END);

	xc->v_idx = -1;
	xc->p_idx = -1;
	xc->f_idx = -1;
}

#ifndef LIBXMP_CORE_DISABLE_IT

static void reset_envelopes_carry(struct context_data *ctx,
				struct channel_data *xc)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct xmp_instrument *xxi;

	if (!IS_VALID_INSTRUMENT(xc->ins))
		return;

 	RESET_NOTE(NOTE_ENV_END);

	xxi = get_instrument(ctx, xc->ins);

	/* Reset envelope positions */
	if (~xxi->aei.flg & XMP_ENVELOPE_CARRY) {
		xc->v_idx = -1;
	}
	if (~xxi->pei.flg & XMP_ENVELOPE_CARRY) {
		xc->p_idx = -1;
	}
	if (~xxi->fei.flg & XMP_ENVELOPE_CARRY) {
		xc->f_idx = -1;
	}
}

#endif

static void set_effect_defaults(struct context_data *ctx, int note,
				struct xmp_subinstrument *sub,
				struct channel_data *xc, int is_toneporta)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct smix_data *smix = &ctx->smix;
	
	if (sub != NULL && note >= 0) {
		struct xmp_instrument *xxi;

		if (xc->ins >= mod->ins) {
			xxi = &smix->xxi[xc->ins - mod->ins];
		} else {
			xxi = &mod->xxi[xc->ins];
		}

		xc->finetune = sub->fin;
		xc->gvl = sub->gvl;

		if (sub->ifc & 0x80) {
			xc->filter.cutoff = (sub->ifc - 0x80) * 2;
		} else if (~xxi->fei.flg & XMP_ENVELOPE_FLT) {
			xc->filter.cutoff = 0xff;
		}
		xc->filter.envelope = 0x100;

		if (sub->ifr & 0x80) {
			xc->filter.resonance = (sub->ifr - 0x80) * 2;
		} /* else {
			xc->filter.resonance = 0;
		} */

		set_lfo_depth(&xc->insvib.lfo, sub->vde);
		set_lfo_rate(&xc->insvib.lfo, sub->vra >> 2);
		set_lfo_waveform(&xc->insvib.lfo, sub->vwf);
		xc->insvib.sweep = sub->vsw;

		set_lfo_phase(&xc->vibrato.lfo, 0);
		set_lfo_phase(&xc->tremolo.lfo, 0);
	}

	xc->delay = 0;
	xc->tremor.val = 0;

	/* Reset arpeggio */
	xc->arpeggio.val[0] = 0;
	xc->arpeggio.count = 0;
	xc->arpeggio.size = 1;
}

static void set_period(struct context_data *ctx, int note,
				struct xmp_subinstrument *sub,
				struct channel_data *xc, int is_toneporta)
{
	struct module_data *m = &ctx->m;

	if (sub != NULL && note >= 0) {
		xc->porta.target = note_to_period(note, xc->finetune,
				HAS_QUIRK(QUIRK_LINEAR), xc->per_adj);
		if (xc->period < 1 || !is_toneporta) {
			xc->period = xc->porta.target;
		}
	}
}

/* From OpenMPT Porta-Pickup.xm:
 * "An instrument number should not reset the current portamento target. The
 *  portamento target is valid until a new target is specified by combining a
 *  note and a portamento effect."
 */
static void set_period_ft2(struct context_data *ctx, int note,
				struct xmp_subinstrument *sub,
				struct channel_data *xc, int is_toneporta)
{
	struct module_data *m = &ctx->m;

	if ((note > 0 && is_toneporta) || xc->porta.target == 0) {
		xc->porta.target = note_to_period(note, xc->finetune,
				HAS_QUIRK(QUIRK_LINEAR), xc->per_adj);
	}
	if (sub != NULL && note >= 0) {
		if (xc->period < 1 || !is_toneporta) {
			xc->period = note_to_period(note, xc->finetune,
				HAS_QUIRK(QUIRK_LINEAR), xc->per_adj);
		}
	}
}


#ifndef LIBXMP_CORE_PLAYER
#define IS_TONEPORTA(x) ((x) == FX_TONEPORTA || (x) == FX_TONE_VSLIDE \
		|| (x) == FX_PER_TPORTA)
#else
#define IS_TONEPORTA(x) ((x) == FX_TONEPORTA || (x) == FX_TONE_VSLIDE)
#endif

#define set_patch(ctx,chn,ins,smp,note) \
	virt_setpatch(ctx, chn, ins, smp, note, 0, 0, 0)

static int read_event_mod(struct context_data *ctx, struct xmp_event *e, int chn)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct channel_data *xc = &p->xc_data[chn];
	int note;
	struct xmp_subinstrument *sub;
	int new_invalid_ins = 0;
	int is_toneporta;
	int use_ins_vol;

	xc->flags = 0;
	note = -1;
	is_toneporta = 0;
	use_ins_vol = 0;

	if (IS_TONEPORTA(e->fxt) || IS_TONEPORTA(e->f2t)) {
		is_toneporta = 1;
	}

	/* Check instrument */

	if (e->ins) {
		int ins = e->ins - 1;
		use_ins_vol = 1;
		SET(NEW_INS);
		xc->fadeout = 0x10000;	/* for painlace.mod pat 0 ch 3 echo */
		xc->per_flags = 0;
		xc->offset.val = 0;
		RESET_NOTE(NOTE_RELEASE|NOTE_FADEOUT);

		if (IS_VALID_INSTRUMENT(ins)) {
			if (is_toneporta) {
				/* Get new instrument volume */
				sub = get_subinstrument(ctx, ins, e->note - 1);
				if (sub != NULL) {
					/* Dennis Lindroos: instrument volume
					 * is not used on split channels
					 */
					if (!xc->split) {
						xc->volume = sub->vol;
					}
					use_ins_vol = 0;
				}
			} else {
				xc->ins = ins;
				xc->ins_fade = mod->xxi[ins].rls;
			}
		} else {
			new_invalid_ins = 1;
			virt_resetchannel(ctx, chn);
		}
	}

	/* Check note */

	if (e->note) {
		SET(NEW_NOTE);

		if (e->note == XMP_KEY_OFF) {
			SET_NOTE(NOTE_RELEASE);
			use_ins_vol = 0;
		} else if (!is_toneporta) {
			xc->key = e->note - 1;
			RESET_NOTE(NOTE_END);
	
			sub = get_subinstrument(ctx, xc->ins, xc->key);
	
			if (!new_invalid_ins && sub != NULL) {
				int transp = mod->xxi[xc->ins].map[xc->key].xpo;
				int smp;
	
				note = xc->key + sub->xpo + transp;
				smp = sub->sid;
	
				if (mod->xxs[smp].len == 0) {
					smp = -1;
				}
	
				if (smp >= 0 && smp < mod->smp) {
					set_patch(ctx, chn, xc->ins, smp, note);
					xc->smp = smp;
				}
			} else {
				xc->flags = 0;
				use_ins_vol = 0;
			}
		}
	}

	sub = get_subinstrument(ctx, xc->ins, xc->key);

	set_effect_defaults(ctx, note, sub, xc, is_toneporta);
	if (e->ins && sub != NULL) {
		reset_envelopes(ctx, xc);
	}

	/* Process new volume */
	if (e->vol) {
		xc->volume = e->vol - 1;
		SET(NEW_VOL);
	}

	/* Secondary effect handled first */
	process_fx(ctx, xc, chn, e, 1);
	process_fx(ctx, xc, chn, e, 0);
	set_period(ctx, note, sub, xc, is_toneporta);

	if (sub == NULL) {
		return 0;
	}

	if (note >= 0) {
		xc->note = note;
		virt_voicepos(ctx, chn, xc->offset.val);
	}

	if (TEST(OFFSET)) {
		if (HAS_QUIRK(QUIRK_PROTRACK) || p->flags & XMP_FLAGS_FX9BUG) {
			xc->offset.val += xc->offset.val2;
		}
		RESET(OFFSET);
	}

	if (use_ins_vol && !TEST(NEW_VOL) && !xc->split) {
		xc->volume = sub->vol;
	}

	return 0;
}

static int sustain_check(struct xmp_envelope *env, int idx)
{
	return (env &&
		(env->flg & XMP_ENVELOPE_ON) &&
		(~env->flg & XMP_ENVELOPE_LOOP) &&
		idx == env->data[env->sus << 1]);
}

static int read_event_ft2(struct context_data *ctx, struct xmp_event *e, int chn)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct channel_data *xc = &p->xc_data[chn];
	int note, key, ins;
	struct xmp_subinstrument *sub;
	int new_invalid_ins;
	int is_toneporta;
	int use_ins_vol;
	int k00 = 0;
	struct xmp_event ev;

	/* From the OpenMPT DelayCombination.xm test case:
         * "Naturally, Fasttracker 2 ignores notes next to an out-of-range
	 *  note delay. However, to check whether the delay is out of range,
	 *  it is simply compared against the current song speed, not taking
	 *  any pattern delays into account."
	 */
	if (p->frame >= p->speed) {
		return 0;
	}

	memcpy(&ev, e, sizeof (struct xmp_event));

	xc->flags = 0;
	note = -1;
	key = ev.note;
	ins = ev.ins;
	new_invalid_ins = 0;
	is_toneporta = 0;
	use_ins_vol = 0;

	/* From the OpenMPT key_off.xm test case:
	 * "Key off at tick 0 (K00) is very dodgy command. If there is a note
	 *  next to it, the note is ignored. If there is a volume column
	 *  command or instrument next to it and the current instrument has
	 *  no volume envelope, the note is faded out instead of being cut."
	 */
	if (ev.fxt == FX_KEYOFF && ev.fxp == 0) {
		k00 = 1;
		key = 0;

		if (ins || ev.vol || ev.f2t) {
			if (IS_VALID_INSTRUMENT(xc->ins) &&
			    ~mod->xxi[xc->ins].aei.flg & XMP_ENVELOPE_ON) {
				SET_NOTE(NOTE_FADEOUT);
				ev.fxt = 0;
			}
		}
	}

	if (IS_TONEPORTA(ev.fxt) || IS_TONEPORTA(ev.f2t)) {
		is_toneporta = 1;
	}

	/* Check instrument */

	/* Ignore invalid instruments. The last instrument, invalid or
	 * not, is preserved in channel data (see read_event() below).
	 * Fixes stray delayed notes in forgotten_city.xm.
	 */
	if (ins > 0 && !IS_VALID_INSTRUMENT(ins - 1)) {
		ins = 0;
	}

	/* FT2: Retrieve old instrument volume */
	if (ins) {
		if (key == 0 || key >= XMP_KEY_OFF) {
			struct xmp_subinstrument *sub;

			/* Previous instrument */
			sub = get_subinstrument(ctx, xc->ins, xc->key);

			/* No note */
			if (sub != NULL) {
				xc->volume = sub->vol;
				xc->pan.val = sub->pan;
				xc->ins_fade = mod->xxi[xc->ins].rls;
				SET(NEW_VOL);
			}
		}
	}

	/* Do this regardless if the instrument is invalid or not */
	if (ev.ins) {
		SET(NEW_INS);
		use_ins_vol = 1;
		xc->fadeout = 0x10000;
		xc->per_flags = 0;
		RESET_NOTE(NOTE_RELEASE|NOTE_SUSEXIT);
		if (!k00) {
			RESET_NOTE(NOTE_FADEOUT);
		}

		if (IS_VALID_INSTRUMENT(ins - 1)) {
			if (!is_toneporta)
				xc->ins = ins - 1;
		} else {
			new_invalid_ins = 1;

			/* If no note is set FT2 doesn't cut on invalid
			 * instruments (it keeps playing the previous one).
			 * If a note is set it cuts the current sample.
			 */
			xc->flags = 0;

			if (is_toneporta) {
				key = 0;
			}
		}
	}

	/* Check note */

	if (ins) {
		if (key > 0 && key < XMP_KEY_OFF) {
			struct xmp_subinstrument *sub;

			/* Retrieve volume when we have note */

			/* and only if we have instrument, otherwise we're in
			 * case 1: new note and no instrument
			 */

			/* Current instrument */
			sub = get_subinstrument(ctx, xc->ins, key - 1);
			if (sub != NULL) {
				xc->volume = sub->vol;
				xc->pan.val = sub->pan;
				xc->ins_fade = mod->xxi[xc->ins].rls;
			} else {
				xc->volume = 0;
			}
			SET(NEW_VOL);
		}
	}

	if (key) {
		SET(NEW_NOTE);

		if (key == XMP_KEY_OFF) {
			int env_on = 0;
			int vol_set = ev.vol != 0 || ev.fxt == FX_VOLSET;
			int delay_fx = ev.fxt == FX_EXTENDED && ev.fxp == 0xd0;
			struct xmp_envelope *env = NULL;

			/* OpenMPT NoteOffVolume.xm:
			 * "If an instrument has no volume envelope, a note-off
			 *  command should cut the sample completely - unless
			 *  there is a volume command next it. This applies to
			 *  both volume commands (volume and effect column)."
			 *
			 * ...and unless we have a keyoff+delay without setting
			 * an instrument. See OffDelay.xm.
			 */
			if (IS_VALID_INSTRUMENT(xc->ins)) {
				env = &mod->xxi[xc->ins].aei;
				if (env->flg & XMP_ENVELOPE_ON) {
					env_on = 1;
				}
			}
			
			if (env_on || (!vol_set && (!ev.ins || !delay_fx))) {
				if (sustain_check(env, xc->v_idx)) {
					/* See OpenMPT EnvOff.xm. In certain
					 * cases a release event is effective
					 * only in the next frame
					 */
					SET_NOTE(NOTE_SUSEXIT);
				} else {
					SET_NOTE(NOTE_RELEASE);
				}
				use_ins_vol = 0;
			} else {
				SET_NOTE(NOTE_FADEOUT);
			}

			/* See OpenMPT keyoff+instr.xm, pattern 2 row 0x40 */
			if (env_on && ev.fxt == FX_EXTENDED &&
			    (ev.fxp >> 4) == EX_DELAY) {
				/* See OpenMPT OffDelay.xm test case */
				if ((ev.fxp & 0xf) != 0) {
					RESET_NOTE(NOTE_RELEASE|NOTE_SUSEXIT);
				}
			}
		} else if (key == XMP_KEY_FADE) {
			/* Handle keyoff + instrument case (NoteOff2.xm) */
			SET_NOTE(NOTE_FADEOUT);
		} else if (is_toneporta) {
			/* set key to 0 so we can have the tone portamento from
			 * the original note (see funky_stars.xm pos 5 ch 9)
			 */
			key = 0;

			/* And do the same if there's no keyoff (see comic
			 * bakery remix.xm pos 1 ch 3)
			 */
		}

		if (ev.ins == 0 && !IS_VALID_INSTRUMENT(xc->old_ins - 1)) {
			new_invalid_ins = 1;
		}

		if (new_invalid_ins) {
			virt_resetchannel(ctx, chn);
		}
	}


	/* Check note range -- from the OpenMPT test NoteLimit.xm:
	 * "I think one of the first things Fasttracker 2 does when parsing a
	 *  pattern cell is calculating the “real” note (i.e. pattern note +
	 *  sample transpose), and if this “real” note falls out of its note
	 *  range, it is ignored completely (wiped from its internal channel
	 *  memory). The instrument number next it, however, is not affected
	 *  and remains in the memory."
	 */
	if (is_valid_note(key - 1)) {
		int k = key - 1;
		sub = get_subinstrument(ctx, xc->ins, k);
		if (!new_invalid_ins && sub != NULL) {
			int transp = mod->xxi[xc->ins].map[k].xpo;
			int k2 = k + sub->xpo + transp;
			if (k2 < 12 || k2 > 130) {
				key = 0;
				RESET(NEW_NOTE);
			}
		}
	}

	if (is_valid_note(key - 1)) {
		xc->key = --key;
		xc->fadeout = 0x10000;
		RESET_NOTE(NOTE_END);

		if (sub != NULL) {
			if (~mod->xxi[xc->ins].aei.flg & XMP_ENVELOPE_ON) {
				RESET_NOTE(NOTE_RELEASE|NOTE_FADEOUT);
			}
		}

		if (!new_invalid_ins && sub != NULL) {
			int transp = mod->xxi[xc->ins].map[key].xpo;
			int smp;

			note = key + sub->xpo + transp;
			smp = sub->sid;

			if (mod->xxs[smp].len == 0) {
				smp = -1;
			}

			if (smp >= 0 && smp < mod->smp) {
				set_patch(ctx, chn, xc->ins, smp, note);
				xc->smp = smp;
			}
		} else {
			xc->flags = 0;
			use_ins_vol = 0;
		}
	}

	sub = get_subinstrument(ctx, xc->ins, xc->key);

	set_effect_defaults(ctx, note, sub, xc, is_toneporta);

	if (ins && sub != NULL && !k00) {
		/* Reset envelopes on new instrument, see olympic.xm pos 10
		 * But make sure we have an instrument set, see Letting go
		 * pos 4 chn 20
		 */
		reset_envelopes(ctx, xc);
	}

	/* Process new volume */
	if (ev.vol) {
		xc->volume = ev.vol - 1;
		SET(NEW_VOL);
		if (TEST_NOTE(NOTE_END)) {	/* m5v-nine.xm */
			xc->fadeout = 0x10000;	/* OpenMPT NoteOff.xm */
			RESET_NOTE(NOTE_RELEASE|NOTE_FADEOUT);
		}
	}

	/* FT2: always reset sample offset */
	xc->offset.val = 0;

	/* Secondary effect handled first */
	process_fx(ctx, xc, chn, &ev, 1);
	process_fx(ctx, xc, chn, &ev, 0);
	set_period_ft2(ctx, note, sub, xc, is_toneporta);

	if (sub == NULL) {
		return 0;
	}

	if (note >= 0) {
		xc->note = note;

		/* From the OpenMPT test cases (3xx-no-old-samp.xm):
		 * "An offset effect that points beyond the sample end should
		 *  stop playback on this channel."
		 */

		if (xc->offset.val >= mod->xxs[sub->sid].len) {
			virt_resetchannel(ctx, chn);
		} else {

			/* (From Decibelter - Cosmic 'Wegian Mamas.xm p04 ch7)
			 * We retrigger the sample only if we have a new note
			 * without tone portamento, otherwise we won't play
			 * sweeps and loops correctly.
			 */
			virt_voicepos(ctx, chn, xc->offset.val);
		}
	}

	if (use_ins_vol && !TEST(NEW_VOL)) {
		xc->volume = sub->vol;
	}

	return 0;
}

static int read_event_st3(struct context_data *ctx, struct xmp_event *e, int chn)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct channel_data *xc = &p->xc_data[chn];
	int note;
	struct xmp_subinstrument *sub;
	int not_same_ins;
	int is_toneporta;
	int use_ins_vol;

	xc->flags = 0;
	note = -1;
	not_same_ins = 0;
	is_toneporta = 0;
	use_ins_vol = 0;

	if (IS_TONEPORTA(e->fxt) || IS_TONEPORTA(e->f2t)) {
		is_toneporta = 1;
	}

	if (virt_mapchannel(ctx, chn) < 0 && xc->ins != e->ins - 1) {
		is_toneporta = 0;
	}

	/* Check instrument */

	if (e->ins) {
		int ins = e->ins - 1;
		SET(NEW_INS);
		use_ins_vol = 1;
		xc->fadeout = 0x10000;
		xc->per_flags = 0;
		xc->offset.val = 0;
		RESET_NOTE(NOTE_RELEASE|NOTE_FADEOUT);

		if (IS_VALID_INSTRUMENT(ins)) {
			/* valid ins */
			if (xc->ins != ins) {
				not_same_ins = 1;
				if (!is_toneporta) {
					xc->ins = ins;
					xc->ins_fade = mod->xxi[ins].rls;
				} else {
					/* Get new instrument volume */
					sub = get_subinstrument(ctx, ins, e->note - 1);
					if (sub != NULL) {
						xc->volume = sub->vol;
						use_ins_vol = 0;
					}
				}
			}
		} else {
			/* invalid ins */

			/* Ignore invalid instruments */
			xc->flags = 0;
			use_ins_vol = 0;
		}
	}

	/* Check note */

	if (e->note) {
		SET(NEW_NOTE);

		if (e->note == XMP_KEY_OFF) {
			SET_NOTE(NOTE_RELEASE);
			use_ins_vol = 0;
		} else if (is_toneporta) {
			/* Always retrig in tone portamento: Fix portamento in
			 * 7spirits.s3m, mod.Biomechanoid
			 */
			if (not_same_ins) {
				xc->offset.val = 0;
			}
		} else {
			xc->key = e->note - 1;
			RESET_NOTE(NOTE_END);
	
			sub = get_subinstrument(ctx, xc->ins, xc->key);
	
			if (sub != NULL) {
				int transp = mod->xxi[xc->ins].map[xc->key].xpo;
				int smp;
	
				note = xc->key + sub->xpo + transp;
				smp = sub->sid;
	
				if (mod->xxs[smp].len == 0) {
					smp = -1;
				}
	
				if (smp >= 0 && smp < mod->smp) {
					set_patch(ctx, chn, xc->ins, smp, note);
					xc->smp = smp;
				}
			} else {
				xc->flags = 0;
				use_ins_vol = 0;
			}
		}
	}

	sub = get_subinstrument(ctx, xc->ins, xc->key);

	set_effect_defaults(ctx, note, sub, xc, is_toneporta);
	if (e->ins && sub != NULL) {
		reset_envelopes(ctx, xc);
	}

	/* Process new volume */
	if (e->vol) {
		xc->volume = e->vol - 1;
		SET(NEW_VOL);
	}

	/* Secondary effect handled first */
	process_fx(ctx, xc, chn, e, 1);
	process_fx(ctx, xc, chn, e, 0);
	set_period(ctx, note, sub, xc, is_toneporta);

	if (sub == NULL) {
		return 0;
	}

	if (note >= 0) {
		xc->note = note;
		virt_voicepos(ctx, chn, xc->offset.val);
	}

	if (use_ins_vol && !TEST(NEW_VOL)) {
		xc->volume = sub->vol;
	}

	/* ST3: check QUIRK_ST3GVOL only in ST3 event reader */
	if (HAS_QUIRK(QUIRK_ST3GVOL) && TEST(NEW_VOL)) {
		xc->volume = xc->volume * p->gvol / m->volbase;
	}

	return 0;
}

#ifndef LIBXMP_CORE_DISABLE_IT

static int check_fadeout(struct context_data *ctx, struct channel_data *xc, int ins)
{
	struct xmp_instrument *xxi = get_instrument(ctx, ins);

	if (xxi == NULL) {
		return 1;
	}

	return (~xxi->aei.flg & XMP_ENVELOPE_ON ||
		~xxi->aei.flg & XMP_ENVELOPE_CARRY ||
		xc->ins_fade == 0 ||
		xc->fadeout <= xc->ins_fade);
}

static int check_invalid_sample(struct context_data *ctx, int ins, int key)
{
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;

	if (ins < mod->ins) {
		int smp = mod->xxi[ins].map[key].ins;
		if (smp == 0xff || smp >= mod->smp) {
			return 1;
		};
	}

	return 0;
}

static void fix_period(struct context_data *ctx, int chn, struct xmp_subinstrument *sub)
{
	if (sub->nna == XMP_INST_NNA_CONT) {
		struct player_data *p = &ctx->p;
		struct module_data *m = &ctx->m;
		struct channel_data *xc = &p->xc_data[chn];
		struct xmp_instrument *xxi = get_instrument(ctx, xc->ins);

		xc->period = note_to_period(xc->key + sub->xpo +
			xxi->map[xc->key_porta].xpo, xc->finetune,
			HAS_QUIRK(QUIRK_LINEAR), xc->per_adj);
	}
}

static int is_same_sid(struct context_data *ctx, int chn, int ins, int key)
{
	struct player_data *p = &ctx->p;
	struct channel_data *xc = &p->xc_data[chn];
	struct xmp_subinstrument *s1, *s2;

	s1 = get_subinstrument(ctx, ins, key);
	s2 = get_subinstrument(ctx, xc->ins, xc->key);

	return (s1 && s2 && s1->sid == s2->sid);
}

static int read_event_it(struct context_data *ctx, struct xmp_event *e, int chn)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct channel_data *xc = &p->xc_data[chn];
	int note, key;
	struct xmp_subinstrument *sub;
	int not_same_ins, not_same_smp;
	int new_invalid_ins;
	int is_toneporta, is_release;
	int candidate_ins;
	int reset_env;
	int use_ins_vol;
	int sample_mode;
	int toneporta_offset;
	struct xmp_event ev;

	memcpy(&ev, e, sizeof (struct xmp_event));

	/* Emulate Impulse Tracker "always read instrument" bug */
	if (ev.ins) {
		xc->delayed_ins = 0;
	} else if (ev.note && xc->delayed_ins) {
		ev.ins = xc->delayed_ins;
		xc->delayed_ins = 0;
	}

	/* Keyoff + instrument retrigs current instrument in old fx mode */
	if (HAS_QUIRK(QUIRK_ITOLDFX)) {
		if (ev.note == XMP_KEY_OFF && IS_VALID_INSTRUMENT(ev.ins -1)) {
			ev.note = xc->key + 1;
			ev.ins = xc->ins + 1;
		}
	}

	xc->flags = 0;
	note = -1;
	key = ev.note;
	not_same_ins = 0;
	not_same_smp = 0;
	new_invalid_ins = 0;
	is_toneporta = 0;
	is_release = 0;
	reset_env = 0;
	use_ins_vol = 0;
	candidate_ins = xc->ins;
	sample_mode = !HAS_QUIRK(QUIRK_VIRTUAL);
	toneporta_offset = 0;

	/* Notes with unmapped instruments are ignored */
	if (ev.ins) {
		if (ev.ins <= mod->ins && has_note_event(&ev)) {
			int ins = ev.ins - 1;
			if (check_invalid_sample(ctx, ins, ev.note - 1)) {
				candidate_ins = ins;
				memset(&ev, 0, sizeof (ev));
			}
		}
	} else {
		if (has_note_event(&ev)) {
			int ins = xc->old_ins - 1;
			if (!IS_VALID_INSTRUMENT(ins)) {
				new_invalid_ins = 1;
			} else if (check_invalid_sample(ctx, ins, ev.note - 1)) {
				memset(&ev, 0, sizeof (ev));
			}
		}
	}

	if (IS_TONEPORTA(ev.fxt) || IS_TONEPORTA(ev.f2t)) {
		is_toneporta = 1;
	}

	if (TEST_NOTE(NOTE_RELEASE | NOTE_FADEOUT)) {
		is_release = 1;
	}

	if (xc->period <= 0 || TEST_NOTE(NOTE_END)) {
		is_toneporta = 0;
	}

	/* Off-Porta.it */
	if (is_toneporta && ev.fxt == FX_OFFSET) {
		is_toneporta = 0;
 		if (!HAS_QUIRK(QUIRK_PRENV)) {
			toneporta_offset = 1;
			RESET_NOTE(NOTE_ENV_END);
		}
	}

	/* Check instrument */

	if (ev.ins) {
		int ins = ev.ins - 1;
		int set_new_ins = 1;

		/* portamento_after_keyoff.it test case */
		if (is_release && is_toneporta && !key) {
			if (HAS_QUIRK(QUIRK_PRENV) || TEST_NOTE(NOTE_SET)) {
				is_toneporta = 0;
				reset_envelopes_carry(ctx, xc);
			}
		}

		if (is_toneporta && xc->ins == ins) {
			if (!HAS_QUIRK(QUIRK_PRENV)) {
				if (is_same_sid(ctx, chn, ins, key - 1)) {
					/* same instrument and same sample */
					set_new_ins = !is_release;
				} else {
					/* same instrument, different sample */
					not_same_ins = 1; /* need this too */
					not_same_smp = 1;
				}
			}
		}

		if (set_new_ins) {
			SET(NEW_INS);
			use_ins_vol = 1;
			reset_env = 1;
		}
		xc->per_flags = 0;

		if (IS_VALID_INSTRUMENT(ins)) {
			/* valid ins */

			if (!key) {
				/* Retrig in new ins in sample mode */
				if (sample_mode && TEST_NOTE(NOTE_END)) {
					virt_voicepos(ctx, chn, 0);
				}

				/* IT: Reset note for every new != ins */
				if (xc->ins == ins) {
					SET(NEW_INS);
					use_ins_vol = 1;
				} else {
					key = xc->key + 1;
				}

				RESET_NOTE(NOTE_SET);
			}

			if (xc->ins != ins && (!is_toneporta || !HAS_QUIRK(QUIRK_PRENV))) {
				candidate_ins = ins;

				if (!is_same_sid(ctx, chn, ins, key - 1)) {
					not_same_ins = 1;
					if (is_toneporta) {
						/* Get new instrument volume */
						sub = get_subinstrument(ctx, ins, key);
						if (sub != NULL) {
							xc->volume = sub->vol;
							use_ins_vol = 0;
						}
					}
				}
			}
		} else {
			/* In sample mode invalid ins cut previous ins */
			if (sample_mode) {
				xc->volume = 0;
			}

			/* Ignore invalid instruments */
			new_invalid_ins = 1;
			xc->flags = 0;
			use_ins_vol = 0;
		}
	}

	/* Check note */

	if (key && !new_invalid_ins) {
		SET(NEW_NOTE);
		SET_NOTE(NOTE_SET);

		if (key == XMP_KEY_FADE) {
			SET_NOTE(NOTE_FADEOUT);
			reset_env = 0;
			use_ins_vol = 0;
		} else if (key == XMP_KEY_CUT) {
			SET_NOTE(NOTE_END | NOTE_CUT);
			xc->period = 0;
			virt_resetchannel(ctx, chn);
		} else if (key == XMP_KEY_OFF) {
			struct xmp_envelope *env = NULL;
			if (IS_VALID_INSTRUMENT(xc->ins)) {
				env = &mod->xxi[xc->ins].aei;
			}
			if (sustain_check(env, xc->v_idx)) {
				SET_NOTE(NOTE_SUSEXIT);
			} else {
				SET_NOTE(NOTE_RELEASE);
			}
			SET(KEY_OFF);
			reset_env = 0;
			use_ins_vol = 0;
		} else {
			/* portamento_after_keyoff.it test case */
			reset_env = 1;

			if (is_toneporta) {
				/* Always retrig on tone portamento: Fix
				 * portamento in 7spirits.s3m, mod.Biomechanoid
			 	 */
				if (not_same_ins || TEST_NOTE(NOTE_END)) {
					SET(NEW_INS);
					RESET_NOTE(NOTE_RELEASE|NOTE_SUSEXIT|NOTE_FADEOUT);
				} else {
					if (is_valid_note(key - 1)) {
						xc->key_porta = key - 1;
					}
					key = 0;
				}
			}
		}
	}

	if (is_valid_note(key - 1) && !new_invalid_ins) {
		xc->key = --key;
		RESET_NOTE(NOTE_END);

		sub = get_subinstrument(ctx, candidate_ins, key);

		if (sub != NULL) {
			int transp = mod->xxi[candidate_ins].map[key].xpo;
			int smp, to;

			note = key + sub->xpo + transp;
			smp = sub->sid;
			if (smp >= mod->smp || mod->xxs[smp].len == 0) {
				smp = -1;
			}

			if (not_same_smp) {
				fix_period(ctx, chn, sub);
				virt_resetchannel(ctx, chn);
			}
			to = virt_setpatch(ctx, chn, candidate_ins, smp,
				note, sub->nna, sub->dct, sub->dca);

			if (to < 0)
				return -1;
			if (to != chn) {
				copy_channel(p, to, chn);
				p->xc_data[to].flags = 0;
			}

			if (smp >= 0) {		/* Not sure if needed */
				xc->smp = smp;
			}
		} else {
			xc->flags = 0;
			use_ins_vol = 0;
		}
	}

	/* Do after virtual channel copy */
	if (is_toneporta) {
		if (HAS_QUIRK(QUIRK_PRENV) && ev.ins) {
			reset_envelopes_carry(ctx, xc);
		}
	}

	if (IS_VALID_INSTRUMENT(candidate_ins)) {
		if (xc->ins != candidate_ins) {
			/* Reset envelopes if instrument changes */
			reset_envelopes(ctx, xc);
		}
		xc->ins = candidate_ins;
		xc->ins_fade = mod->xxi[candidate_ins].rls;
	}

	/* Reset in case of new instrument and the previous envelope has
	 * finished (OpenMPT test EnvReset.it). This must take place after
	 * channel copies in case of NNA (see test/test.it)
	 * Also if we have envelope in carry mode, check fadeout
	 */
	if (ev.ins && TEST_NOTE(NOTE_ENV_END)) {
		if (check_fadeout(ctx, xc, candidate_ins)) {
			reset_envelopes(ctx, xc);
		} else {
			reset_env = 0;
		}
	}

	if (reset_env) {
		if (ev.note) {
			RESET_NOTE(NOTE_RELEASE|NOTE_SUSEXIT|NOTE_FADEOUT);
		}
		/* Set after copying to new virtual channel (see ambio.it) */
		xc->fadeout = 0x10000;
	}


	sub = get_subinstrument(ctx, xc->ins, xc->key);

	set_effect_defaults(ctx, note, sub, xc, is_toneporta);
	if (sub != NULL) {
		if (note >= 0) {
			int pan_swing = (sub->rvv & 0xff00) >> 8;
			CLAMP(pan_swing, 0, 64);

			if (sub->pan >= 0) {
				xc->pan.val = sub->pan;
				xc->pan.surround = 0;
			}

			if (TEST_NOTE(NOTE_CUT)) {
				reset_envelopes(ctx, xc);
			} else if (!toneporta_offset) {
				reset_envelopes_carry(ctx, xc);
			}
			RESET_NOTE(NOTE_CUT);

			if (pan_swing) {
				xc->pan.val = (xc->pan.val * (64 - pan_swing) +
					(rand() % 256) * pan_swing) >> 6;
			}
		} else if (ev.ins) {
			if (sub->pan >= 0)
				xc->pan.val = sub->pan;
		}
	}
	
	/* Process new volume */
	if (ev.vol && (!TEST_NOTE(NOTE_CUT) || ev.ins != 0)) {
		xc->volume = ev.vol - 1;
		SET(NEW_VOL);
	}

	/* IT: always reset sample offset */
	xc->offset.val &= ~0xffff;

	/* According to Storlek test 25, Impulse Tracker handles the volume
	 * column effects after the standard effects.
	 */
	process_fx(ctx, xc, chn, &ev, 0);
	process_fx(ctx, xc, chn, &ev, 1);
	set_period(ctx, note, sub, xc, is_toneporta);

	if (sub == NULL) {
		return 0;
	}

	if (note >= 0) {
		xc->note = note;
		virt_voicepos(ctx, chn, xc->offset.val);
	}

	if (use_ins_vol && !TEST(NEW_VOL)) {
		int vol_swing = sub->rvv & 0xff;
		CLAMP(vol_swing, 0, 100);

		xc->volume = sub->vol;

		if (vol_swing) {
			xc->volume = (xc->volume * (100 - vol_swing) + 
				(rand() % (xc->volume + 1)) * vol_swing) / 100;
		}
	}

	return 0;
}

#endif

#ifndef LIBXMP_CORE_PLAYER

static int read_event_med(struct context_data *ctx, struct xmp_event *e, int chn)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct channel_data *xc = &p->xc_data[chn];
	int note;
	struct xmp_subinstrument *sub;
	int new_invalid_ins = 0;
	int is_toneporta;
	int use_ins_vol;
	int finetune;

	xc->flags = 0;
	note = -1;
	is_toneporta = 0;
	use_ins_vol = 0;

	if (e->fxt == FX_TONEPORTA || e->fxt == FX_TONE_VSLIDE) {
		is_toneporta = 1;
	}

	/* Check instrument */

	if (e->ins && e->note) {
		int ins = e->ins - 1;
		use_ins_vol = 1;
		SET(NEW_INS);
		xc->fadeout = 0x10000;
		xc->offset.val = 0;
		RESET_NOTE(NOTE_RELEASE|NOTE_FADEOUT);

		if (IS_VALID_INSTRUMENT(ins)) {
			if (is_toneporta) {
				/* Get new instrument volume */
				sub = get_subinstrument(ctx, ins, e->note - 1);
				if (sub != NULL) {
					xc->volume = sub->vol;
					use_ins_vol = 0;
				}
			} else {
				xc->ins = ins;
				xc->ins_fade = mod->xxi[ins].rls;
			}
		} else {
			new_invalid_ins = 1;
			virt_resetchannel(ctx, chn);
		}

		MED_CHANNEL_EXTRAS(*xc)->arp = 0;
		MED_CHANNEL_EXTRAS(*xc)->aidx = 0;
	} else {
		/* Hold */
		if (e->ins && !e->note) {
			use_ins_vol = 1;
		}
	}

	/* Check note */

	if (e->note) {
		SET(NEW_NOTE);

		if (e->note == XMP_KEY_OFF) {
			SET_NOTE(NOTE_RELEASE);
			use_ins_vol = 0;
		} else if (e->note == XMP_KEY_CUT) {
			SET_NOTE(NOTE_END);
			xc->period = 0;
			virt_resetchannel(ctx, chn);
		} else if (!is_toneporta && IS_VALID_INSTRUMENT(xc->ins)) {
			struct xmp_instrument *xxi = &mod->xxi[xc->ins];

			xc->key = e->note - 1;
			RESET_NOTE(NOTE_END);
		
			xc->per_adj = 0.0;
			if (xxi->nsm > 1 && HAS_MED_INSTRUMENT_EXTRAS(*xxi)) {
				/* synth or iffoct */
				if (MED_INSTRUMENT_EXTRAS(*xxi)->vts == 0 &&
				    MED_INSTRUMENT_EXTRAS(*xxi)->wts == 0) {
					/* iffoct */
					xc->per_adj = 2.0;
				}
			}
	
			sub = get_subinstrument(ctx, xc->ins, xc->key);
	
			if (!new_invalid_ins && sub != NULL) {
				int transp = xxi->map[xc->key].xpo;
				int smp;
	
				note = xc->key + sub->xpo + transp;
				smp = sub->sid;
	
				if (mod->xxs[smp].len == 0) {
					smp = -1;
				}
	
				if (smp >= 0 && smp < mod->smp) {
					set_patch(ctx, chn, xc->ins, smp, note);
					xc->smp = smp;
				}
			} else {
				xc->flags = 0;
				use_ins_vol = 0;
			}
		}
	}

	sub = get_subinstrument(ctx, xc->ins, xc->key);

	/* Keep effect-set finetune if no instrument set */
	finetune = xc->finetune;
	set_effect_defaults(ctx, note, sub, xc, is_toneporta);
	if (!e->ins) {
		xc->finetune = finetune;
	}

	if (e->ins && sub != NULL) {
		reset_envelopes(ctx, xc);
	}

	/* Process new volume */
	if (e->vol) {
		xc->volume = e->vol - 1;
		SET(NEW_VOL);
	}

	/* Secondary effect handled first */
	process_fx(ctx, xc, chn, e, 1);
	process_fx(ctx, xc, chn, e, 0);
	set_period(ctx, note, sub, xc, is_toneporta);

	if (sub == NULL) {
		return 0;
	}

	if (note >= 0) {
		xc->note = note;
		virt_voicepos(ctx, chn, xc->offset.val);
	}

	if (use_ins_vol && !TEST(NEW_VOL)) {
		xc->volume = sub->vol;
	}

	return 0;
}

#endif

static int read_event_smix(struct context_data *ctx, struct xmp_event *e, int chn)
{
	struct player_data *p = &ctx->p;
	struct smix_data *smix = &ctx->smix;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct channel_data *xc = &p->xc_data[chn];
	struct xmp_subinstrument *sub;
	int is_smix_ins;
	int ins, note, transp, smp;

	xc->flags = 0;

	if (!e->ins)
		return 0;

	is_smix_ins = 0;
	ins = e->ins - 1;
	SET(NEW_INS);
	xc->fadeout = 0x10000;
	xc->per_flags = 0;
	xc->offset.val = 0;
	RESET_NOTE(NOTE_RELEASE);

	xc->ins = ins;

	if (ins >= mod->ins && ins < mod->ins + smix->ins) {
		is_smix_ins = 1;
		xc->ins_fade = smix->xxi[xc->ins - mod->ins].rls;
	}

	SET(NEW_NOTE);

	if (e->note == XMP_KEY_OFF) {
		SET_NOTE(NOTE_RELEASE);
		return 0;
	}

	xc->key = e->note - 1;
	RESET_NOTE(NOTE_END);

	if (is_smix_ins) {
		sub = &smix->xxi[xc->ins - mod->ins].sub[0];
		if (sub == NULL)
			return 0;

		note = xc->key + sub->xpo;
		smp = sub->sid;
		if (smix->xxs[smp].len == 0)
			smp = -1;
		if (smp >= 0 && smp < smix->smp) {
			smp += mod->smp;
			set_patch(ctx, chn, xc->ins, smp, note);
			xc->smp = smp;
		}
	} else {
		transp = mod->xxi[xc->ins].map[xc->key].xpo;
		sub = get_subinstrument(ctx, xc->ins, xc->key);
		note = xc->key + sub->xpo + transp;
		smp = sub->sid;
		if (mod->xxs[smp].len == 0)
			smp = -1;
		if (smp >= 0 && smp < mod->smp) {
			set_patch(ctx, chn, xc->ins, smp, note);
			xc->smp = smp;
		}
	}

	set_effect_defaults(ctx, note, sub, xc, 0);
	set_period(ctx, note, sub, xc, 0);

	if (e->ins && sub != NULL) {
		reset_envelopes(ctx, xc);
	}

	xc->volume = e->vol - 1;

	xc->note = note;
	virt_voicepos(ctx, chn, xc->offset.val);

	return 0;
}

int read_event(struct context_data *ctx, struct xmp_event *e, int chn)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct channel_data *xc = &p->xc_data[chn];

	if (e->ins != 0)
		xc->old_ins = e->ins;

	if (TEST_NOTE(NOTE_SAMPLE_END)) {
		SET_NOTE(NOTE_END);
	}

	if (chn >= m->mod.chn) {
		return read_event_smix(ctx, e, chn);
	} else switch (m->read_event_type) {
	case READ_EVENT_MOD:
		return read_event_mod(ctx, e, chn);
	case READ_EVENT_FT2:
		return read_event_ft2(ctx, e, chn);
	case READ_EVENT_ST3:
		return read_event_st3(ctx, e, chn);
#ifndef LIBXMP_CORE_DISABLE_IT
	case READ_EVENT_IT:
		return read_event_it(ctx, e, chn);
#endif
#ifndef LIBXMP_CORE_PLAYER
	case READ_EVENT_MED:
		return read_event_med(ctx, e, chn);
#endif
	default:
		return read_event_mod(ctx, e, chn);
	}
}

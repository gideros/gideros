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

#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "virtual.h"
#include "mixer.h"
#include "period.h"
#include "player.h"	/* for set_sample_end() */

#ifndef LIBXMP_CORE_PLAYER
#include "synth.h"
#endif


#define FLAG_16_BITS	0x01
#define FLAG_STEREO	0x02
#define FLAG_FILTER	0x04
#define FLAG_ACTIVE	0x10
#define FLAG_SYNTH	0x20
#define FIDX_FLAGMASK	(FLAG_16_BITS | FLAG_STEREO | FLAG_FILTER)

#define DOWNMIX_SHIFT	 12
#define LIM8_HI		 127
#define LIM8_LO		-128
#define LIM16_HI	 32767
#define LIM16_LO	-32768

#define MIX_FN(x) void x (struct mixer_voice *, int *, int, int, int, int)

MIX_FN(smix_mono_8bit_nearest);
MIX_FN(smix_mono_8bit_linear);
MIX_FN(smix_mono_16bit_nearest);
MIX_FN(smix_mono_16bit_linear);
MIX_FN(smix_stereo_8bit_nearest);
MIX_FN(smix_stereo_8bit_linear);
MIX_FN(smix_stereo_16bit_nearest);
MIX_FN(smix_stereo_16bit_linear);
MIX_FN(smix_mono_8bit_spline);
MIX_FN(smix_mono_16bit_spline);
MIX_FN(smix_stereo_8bit_spline);
MIX_FN(smix_stereo_16bit_spline);

#ifndef LIBXMP_CORE_DISABLE_IT
MIX_FN(smix_mono_8bit_linear_filter);
MIX_FN(smix_mono_16bit_linear_filter);
MIX_FN(smix_stereo_8bit_linear_filter);
MIX_FN(smix_stereo_16bit_linear_filter);
MIX_FN(smix_mono_8bit_spline_filter);
MIX_FN(smix_mono_16bit_spline_filter);
MIX_FN(smix_stereo_8bit_spline_filter);
MIX_FN(smix_stereo_16bit_spline_filter);
#endif


/* Mixers array index:
 *
 * bit 0: 0=8 bit sample, 1=16 bit sample
 * bit 1: 0=mono output, 1=stereo output
 * bit 2: 0=unfiltered, 1=filtered
 */

typedef void (*mixer_set[])(struct mixer_voice *, int *, int, int, int, int);

static mixer_set nearest_mixers = {
	smix_mono_8bit_nearest,
	smix_mono_16bit_nearest,
	smix_stereo_8bit_nearest,
	smix_stereo_16bit_nearest,

#ifndef LIBXMP_CORE_DISABLE_IT
	smix_mono_8bit_nearest,
	smix_mono_16bit_nearest,
	smix_stereo_8bit_nearest,
	smix_stereo_16bit_nearest,
#endif
};

static mixer_set linear_mixers = {
	smix_mono_8bit_linear,
	smix_mono_16bit_linear,
	smix_stereo_8bit_linear,
	smix_stereo_16bit_linear,

#ifndef LIBXMP_CORE_DISABLE_IT
	smix_mono_8bit_linear_filter,
	smix_mono_16bit_linear_filter,
	smix_stereo_8bit_linear_filter,
	smix_stereo_16bit_linear_filter
#endif
};

static mixer_set spline_mixers = {
	smix_mono_8bit_spline,
	smix_mono_16bit_spline,
	smix_stereo_8bit_spline,
	smix_stereo_16bit_spline,

#ifndef LIBXMP_CORE_DISABLE_IT
	smix_mono_8bit_spline_filter,
	smix_mono_16bit_spline_filter,
	smix_stereo_8bit_spline_filter,
	smix_stereo_16bit_spline_filter
#endif
};

/* Downmix 32bit samples to 8bit, signed or unsigned, mono or stereo output */
static void downmix_int_8bit(char *dest, int32 *src, int num, int amp, int offs)
{
	int smp;
	int shift = DOWNMIX_SHIFT + 8 - amp;

	for (; num--; src++, dest++) {
		smp = *src >> shift;
		if (smp > LIM8_HI) {
			*dest = LIM8_HI;
		} else if (smp < LIM8_LO) {
			*dest = LIM8_LO;
		} else {
			*dest = smp;
		}

		if (offs) *dest += offs;
	}
}


/* Downmix 32bit samples to 16bit, signed or unsigned, mono or stereo output */
static void downmix_int_16bit(int16 *dest, int32 *src, int num, int amp, int offs)
{
	int smp;
	int shift = DOWNMIX_SHIFT - amp;

	for (; num--; src++, dest++) {
		smp = *src >> shift;
		if (smp > LIM16_HI) {
			*dest = LIM16_HI;
		} else if (smp < LIM16_LO) {
			*dest = LIM16_LO;
		} else {
			*dest = smp;
		}

		if (offs) *dest += offs;
	}
}


/* Prepare the mixer for the next tick */
void mixer_prepare(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct mixer_data *s = &ctx->s;
	int bytelen;

	s->ticksize = s->freq * m->time_factor * m->rrate / p->bpm / 1000;

	bytelen = s->ticksize * sizeof(int);
	if (~s->format & XMP_FORMAT_MONO) {
		bytelen *= 2;
	}
	memset(s->buf32, 0, bytelen);
}


/* Hipolito's rampdown anticlick */
static void rampdown(struct context_data *ctx, int voc, int32 *buf, int count)
{
	struct player_data *p = &ctx->p;
	struct mixer_data *s = &ctx->s;
	int smp_l, smp_r;
	int dec_l, dec_r;

	if (voc < 0) {
		/* initialize */
		smp_r = s->dtright;
		smp_l = s->dtleft;
	} else {
		struct mixer_voice *vi = &p->virt.voice_array[voc];
		smp_r = vi->sright;
		smp_l = vi->sleft;
		vi->sright = vi->sleft = 0;
	}

	if (smp_l == 0 && smp_r == 0) {
		return;
	}

	if (buf == NULL) {
		buf = s->buf32;
		count = SLOW_RELEASE;
	}

	if (count <= 0) {
		return;
	}

	dec_r = smp_r / count;
	dec_l = smp_l / count;

	while ((smp_r || smp_l) && count--) {
		if (~s->format & XMP_FORMAT_MONO) {
			if (dec_r > 0) {
				if (smp_r > dec_r) {
					smp_r -= dec_r;
					*buf += smp_r;
				} else {
					smp_r = 0;
				}
			} else {
				if (smp_r < dec_r) {
					smp_r -= dec_r;
					*buf += smp_r;
				} else {
					smp_r = 0;
				}
			}
			buf++;
		}

		if (dec_l > 0) {
			if (smp_l > dec_l) {
				smp_l -= dec_l;
				*buf += smp_l;
			} else {
				smp_l = 0;
			}
		} else {
			if (smp_l < dec_l) {
				smp_l -= dec_l;
				*buf += smp_l;
			} else {
				smp_l = 0;
			}
		}
		buf++;
	}
}


/* Ok, it's messy, but it works :-) Hipolito */
static void anticlick(struct context_data *ctx, int voc, int vol, int pan,
		      int32 *buf, int count)
{
	int oldvol, newvol, pan0;
	struct player_data *p = &ctx->p;
	struct mixer_data *s = &ctx->s;
	struct mixer_voice *vi = &p->virt.voice_array[voc];

	/* From: Mirko Buffoni <mirbuf@gmail.com>
	 * To: Claudio Matsuoka <cmatsuoka@gmail.com>
	 * Date: Nov 29, 2007 6:45 AM
	 *  
	 * Put PAN SEPARATION to 100. Then it crashes. Other modules crash when
	 * PAN SEPARATION = 100, (...) moving separation one step behind, stop
	 * crashes.
	 */
	pan0 = vi->pan;
	if (pan0 < -127) {
		pan0 = -127;
	}

	if (vi->vol) {
		oldvol = vi->vol * (0x80 - pan0);
		newvol = vol * (0x80 - pan);
		vi->sright -= (int64)vi->sright * newvol / oldvol;

		oldvol = vi->vol * (0x80 + pan0);
		newvol = vol * (0x80 + pan);
		vi->sleft -= (int64)vi->sleft * newvol / oldvol;
	}

	if (!buf) {
		s->dtright += vi->sright;
		s->dtleft += vi->sleft;
		vi->sright = vi->sleft = 0;
	} else {
		rampdown(ctx, voc, buf, count);
	}
}

static void set_sample_end(struct context_data *ctx, int voc, int end)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct mixer_voice *vi = &p->virt.voice_array[voc];
	struct channel_data *xc;

	if ((uint32)voc >= p->virt.maxvoc)
		return;

	xc = &p->xc_data[vi->chn];

	if (end) {
		SET_NOTE(NOTE_SAMPLE_END);
		if (HAS_QUIRK(QUIRK_RSTCHN)) {
			virt_resetvoice(ctx, voc, 0);
		}
	} else {
		RESET_NOTE(NOTE_SAMPLE_END);
	}
}

/* Fill the output buffer calling one of the handlers. The buffer contains
 * sound for one tick (a PAL frame or 1/50s for standard vblank-timed mods)
 */
void mixer_softmixer(struct context_data *ctx)
{
	struct player_data *p = &ctx->p;
	struct mixer_data *s = &ctx->s;
	struct module_data *m = &ctx->m;
	struct xmp_module *mod = &m->mod;
	struct xmp_sample *xxs;
	struct mixer_voice *vi;
	int samples, size;
	int vol_l, vol_r, step, voc;
	int prev_l, prev_r;
	int lps, lpe;
#ifndef LIBXMP_CORE_PLAYER
	int synth = 1;
#endif
	int32 *buf_pos;
	void (*mix_fn)(struct mixer_voice *, int *, int, int, int, int);
	mixer_set *mixers;

	switch (s->interp) {
	case XMP_INTERP_NEAREST:
		mixers = &nearest_mixers;
		break;
	case XMP_INTERP_LINEAR:
		mixers = &linear_mixers;
		break;
	case XMP_INTERP_SPLINE:
		mixers = &spline_mixers;
		break;
	default:
		mixers = &linear_mixers;
	}

	mixer_prepare(ctx);

	rampdown(ctx, -1, NULL, 0);	/* Anti-click */

	for (voc = 0; voc < p->virt.maxvoc; voc++) {
		vi = &p->virt.voice_array[voc];

		if (vi->chn < 0)
			continue;

		if (vi->period < 1) {
			virt_resetvoice(ctx, voc, 1);
			continue;
		}

		vi->pos0 = vi->pos;

		buf_pos = s->buf32;
		if (vi->pan == PAN_SURROUND) {
			vol_r = vi->vol * 0x80;
			vol_l = -vi->vol * 0x80;
		} else {
			vol_r = vi->vol * (0x80 - vi->pan);
			vol_l = vi->vol * (0x80 + vi->pan);
		}

#ifndef LIBXMP_CORE_PLAYER
		if (vi->fidx & FLAG_SYNTH) {
			if (synth) {
				m->synth->mixer(ctx, buf_pos, s->ticksize,
						vol_l >> 7, vol_r >> 7,
						vi->fidx & FLAG_STEREO);
				synth = 0;
			}
			continue;
		}
#endif

		step = ((int64)s->pbase << 24) / vi->period;

		if (step == 0) {	/* otherwise m5v-nwlf.it crashes */
			continue;
		}

		if (vi->smp < mod->smp) {
			xxs = &mod->xxs[vi->smp];
		} else {
			xxs = &ctx->smix.xxs[vi->smp - mod->smp];
		}

		lps = xxs->lps;
		lpe = xxs->lpe;

		if (p->flags & XMP_FLAGS_FIXLOOP) {
			lps >>= 1;
		}

		for (size = s->ticksize; size > 0; ) {
			int split_noloop = 0;

			if (p->xc_data[vi->chn].split) {
				split_noloop = 1;
			}

			/* How many samples we can write before the loop break
			 * or sample end... */
			if (vi->pos >= vi->end) {
				samples = 0;
			} else {
				int64 s = 1 + (((int64)(vi->end - vi->pos) <<
					SMIX_SHIFT) - vi->frac) / step;
				/* ...inside the tick boundaries */
				if (s > size) {
					s = size;
				}

				samples = s;
			}

			if (vi->vol) {
				int idx;
				int mix_size = samples;
				int mixer = vi->fidx & FIDX_FLAGMASK;

				if (~s->format & XMP_FORMAT_MONO) {
					mix_size *= 2;
				}

				/* For Hipolito's anticlick routine */
				idx = mix_size;
				if (mix_size >= 2) {
					prev_r = buf_pos[idx - 2];
					prev_l = buf_pos[idx - 1];
				} else {
					prev_r = prev_l = 0;
				}

#ifndef LIBXMP_CORE_DISABLE_IT
				/* See OpenMPT env-flt-max.it */
				if (vi->filter.cutoff >= 0xfe &&
                                    vi->filter.resonance == 0) {
					mixer &= ~FLAG_FILTER;
				}
#endif

				mix_fn = (*mixers)[mixer];

				/* Call the output handler */
				if (samples >= 0 && vi->sptr != NULL) {
					mix_fn(vi, buf_pos, samples, vol_l,
								vol_r, step);
					buf_pos += mix_size;

					/* For Hipolito's anticlick routine */
					if (mix_size >= 2) {
						vi->sright = buf_pos[-2] - prev_r;
						vi->sleft = buf_pos[-1] - prev_l;
					}
				}
			}

			vi->frac += step * samples;
			vi->pos += vi->frac >> SMIX_SHIFT;
			vi->frac &= SMIX_MASK;

			/* No more samples in this tick */
			size -= samples;
			if (size <= 0)
				continue;

			/* First sample loop run */
			if ((~xxs->flg & XMP_SAMPLE_LOOP) || split_noloop) {
				anticlick(ctx, voc, 0, 0, buf_pos, size);
				set_sample_end(ctx, voc, 1);
				size = 0;
				continue;
			}

			vi->pos -= lpe - lps;	/* forward loop */
			vi->end = lpe;
			vi->sample_loop = 1;

			if (xxs->flg & XMP_SAMPLE_LOOP_BIDIR) {
				vi->end += lpe - lps;
				vi->pos -= lpe - lps;	/* forward loop */
			}
		}
	}

	/* Render final frame */

	size = s->ticksize;
	if (~s->format & XMP_FORMAT_MONO) {
		size *= 2;
	}

	if (size > XMP_MAX_FRAMESIZE) {
		size = XMP_MAX_FRAMESIZE;
	}

	if (s->format & XMP_FORMAT_8BIT) {
		downmix_int_8bit(s->buffer, s->buf32, size, s->amplify,
				s->format & XMP_FORMAT_UNSIGNED ? 0x80 : 0);
	} else {
		downmix_int_16bit((int16 *)s->buffer, s->buf32, size,s->amplify,
				s->format & XMP_FORMAT_UNSIGNED ? 0x8000 : 0);
	}

	s->dtright = s->dtleft = 0;
}

void mixer_voicepos(struct context_data *ctx, int voc, int pos, int frac)
{
	struct player_data *p = &ctx->p;
	struct module_data *m = &ctx->m;
	struct mixer_voice *vi = &p->virt.voice_array[voc];
	struct xmp_sample *xxs;
	int lps;

	if (vi->smp < m->mod.smp) {
 		xxs = &m->mod.xxs[vi->smp];
	} else {
 		xxs = &ctx->smix.xxs[vi->smp - m->mod.smp];
	}

	if (xxs->flg & XMP_SAMPLE_SYNTH) {
		return;
	}

	if (xxs->flg & XMP_SAMPLE_LOOP) {
		if ((xxs->flg & XMP_SAMPLE_LOOP_FULL) && vi->sample_loop == 0) {
			vi->end = xxs->len;
		} else {
			vi->end = xxs->lpe;
		}
	} else {
		vi->end = xxs->len;
	}

	if (pos >= vi->end) {
		if (xxs->flg & XMP_SAMPLE_LOOP) {
			pos = xxs->lps;
		} else {
			pos = xxs->len;
		}
	}

	vi->pos = pos;
	vi->frac = frac;

	lps = xxs->lps;
	if (p->flags & XMP_FLAGS_FIXLOOP) {
		lps >>= 1;
	}

	if (xxs->flg & XMP_SAMPLE_LOOP_BIDIR) {
		vi->end += (xxs->lpe - lps);
	}

	vi->attack = SLOW_ATTACK;
}

int mixer_getvoicepos(struct context_data *ctx, int voc)
{
	struct player_data *p = &ctx->p;
	struct mixer_voice *vi = &p->virt.voice_array[voc];
	struct xmp_sample *xxs;

	xxs = get_sample(ctx, vi->smp);

	if (xxs->flg & XMP_SAMPLE_SYNTH) {
		return 0;
	}

	if (xxs->flg & XMP_SAMPLE_LOOP_BIDIR) {
		if (vi->pos >= xxs->lpe) {
			return xxs->lpe - (vi->pos - xxs->lpe) - 1;
		}
	}

	return vi->pos;
}

void mixer_setpatch(struct context_data *ctx, int voc, int smp)
{
	struct player_data *p = &ctx->p;
#ifndef LIBXMP_CORE_DISABLE_IT
	struct module_data *m = &ctx->m;
#endif
	struct mixer_data *s = &ctx->s;
	struct mixer_voice *vi = &p->virt.voice_array[voc];
	struct xmp_sample *xxs;

	xxs = get_sample(ctx, smp);

	vi->smp = smp;
	vi->vol = 0;
	vi->pan = 0;
	vi->sample_loop = 0;

	vi->fidx = 0;

	if (~s->format & XMP_FORMAT_MONO) {
		vi->fidx |= FLAG_STEREO;
	}

	set_sample_end(ctx, voc, 0);

#ifndef LIBXMP_CORE_PLAYER
	if (xxs->flg & XMP_SAMPLE_SYNTH) {
		vi->fidx |= FLAG_SYNTH;
		m->synth->setpatch(ctx, voc, xxs->data);
		return;
	}
#endif

	mixer_setvol(ctx, voc, 0);

	vi->sptr = xxs->data;
	vi->fidx |= FLAG_ACTIVE;

#ifndef LIBXMP_CORE_DISABLE_IT
	if (HAS_QUIRK(QUIRK_FILTER) && s->dsp & XMP_DSP_LOWPASS) {
		vi->fidx |= FLAG_FILTER;
	}
#endif

	if (xxs->flg & XMP_SAMPLE_16BIT) {
		vi->fidx |= FLAG_16_BITS;
	}

	mixer_voicepos(ctx, voc, 0, 0);
}

void mixer_setnote(struct context_data *ctx, int voc, int note)
{
	struct player_data *p = &ctx->p;
	struct mixer_voice *vi = &p->virt.voice_array[voc];

	/* FIXME: Workaround for crash on notes that are too high
	 *        see 6nations.it (+114 transposition on instrument 16)
	 */
	if (note > 149) {
		note = 149;
	}

	vi->note = note;
	vi->period = note_to_period_mix(note, 0);
	vi->attack = SLOW_ATTACK;
}

void mixer_setbend(struct context_data *ctx, int voc, int bend)
{
	struct player_data *p = &ctx->p;
	struct mixer_voice *vi = &p->virt.voice_array[voc];
#ifndef LIBXMP_CORE_PLAYER
	struct module_data *m = &ctx->m;
#endif

	vi->period = note_to_period_mix(vi->note, bend);

#ifndef LIBXMP_CORE_PLAYER
	if (vi->fidx & FLAG_SYNTH) {
		m->synth->setnote(ctx, voc, vi->note, bend >> 7);
	}
#endif
}

void mixer_setvol(struct context_data *ctx, int voc, int vol)
{
	struct player_data *p = &ctx->p;
	struct mixer_data *s = &ctx->s;
	struct mixer_voice *vi = &p->virt.voice_array[voc];
#ifndef LIBXMP_CORE_PLAYER
	struct module_data *m = &ctx->m;
#endif

	if (s->interp > XMP_INTERP_NEAREST)
		anticlick(ctx, voc, vol, vi->pan, NULL, 0);

	vi->vol = vol;

#ifndef LIBXMP_CORE_PLAYER
	if (vi->fidx & FLAG_SYNTH) {
		m->synth->setvol(ctx, voc, vol >> 4);
	}
#endif
}

void mixer_seteffect(struct context_data *ctx, int voc, int type, int val)
{
#ifndef LIBXMP_CORE_DISABLE_IT
	struct player_data *p = &ctx->p;
	struct mixer_voice *vi = &p->virt.voice_array[voc];

	switch (type) {
	case DSP_EFFECT_CUTOFF:
		vi->filter.cutoff = val;
		break;
	case DSP_EFFECT_RESONANCE:
		vi->filter.resonance = val;
		break;
	case DSP_EFFECT_FILTER_A0:
		vi->filter.a0 = val;
		break;
	case DSP_EFFECT_FILTER_B0:
		vi->filter.b0 = val;
		break;
	case DSP_EFFECT_FILTER_B1:
		vi->filter.b1 = val;
		break;
	}
#endif
}

void mixer_setpan(struct context_data *ctx, int voc, int pan)
{
	struct player_data *p = &ctx->p;
	struct mixer_voice *vi = &p->virt.voice_array[voc];

	vi->pan = pan;
}

int mixer_numvoices(struct context_data *ctx, int num)
{
	struct mixer_data *s = &ctx->s;

	if (num > s->numvoc || num < 0) {
		return s->numvoc;
	} else {
		return num;
	}
}

int mixer_on(struct context_data *ctx, int rate, int format, int c4rate)
{
	struct mixer_data *s = &ctx->s;

	s->buffer = calloc(2, XMP_MAX_FRAMESIZE);
	if (s->buffer == NULL)
		goto err;

	s->buf32 = calloc(sizeof(int), XMP_MAX_FRAMESIZE);
	if (s->buf32 == NULL)
		goto err1;

	s->freq = rate;
	s->format = format;
	s->amplify = DEFAULT_AMPLIFY;
	s->mix = DEFAULT_MIX;
	s->pbase = SMIX_C4NOTE * c4rate / s->freq;
	s->interp = XMP_INTERP_LINEAR;	/* default interpolation type */
	s->dsp = XMP_DSP_LOWPASS;	/* enable filters by default */
	s->numvoc = SMIX_NUMVOC;
	s->dtright = s->dtleft = 0;

	return 0;

    err1:
	free(s->buffer);
    err:
	return -1;
}

void mixer_off(struct context_data *ctx)
{
	struct mixer_data *s = &ctx->s;

	free(s->buffer);
	free(s->buf32);
	s->buf32 = NULL;
	s->buffer = NULL;
}

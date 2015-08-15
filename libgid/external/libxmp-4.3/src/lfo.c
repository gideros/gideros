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
#include "lfo.h"

#define WAVEFORM_SIZE 64

static const int sine_wave[WAVEFORM_SIZE] = {
	   0,  24,  49,  74,  97, 120, 141, 161, 180, 197, 212, 224,
	 235, 244, 250, 253, 255, 253, 250, 244, 235, 224, 212, 197,
	 180, 161, 141, 120,  97,  74,  49,  24,   0, -24, -49, -74,
	 -97,-120,-141,-161,-180,-197,-212,-224,-235,-244,-250,-253,
	-255,-253,-250,-244,-235,-224,-212,-197,-180,-161,-141,-120,
	 -97, -74, -49, -24
};

/* LFO */

static int get_lfo_mod(struct lfo *lfo, int div)
{
	int val;

	if (lfo->rate == 0 || div == 0)
		return 0;

	switch (lfo->type) {
	case 0: /* sine */
		val = sine_wave[lfo->phase];
		break;
	case 1:	/* ramp down */
		val = 255 - (lfo->phase << 3);
		break;
	case 2:	/* square */
		val = lfo->phase < WAVEFORM_SIZE / 2 ? 255 : -255;
		break;
	case 3: /* random */
		val = ((rand() & 0x1ff) - 256);
		break;
	default:
		return 0;
	}

	return val * lfo->depth / div;
}

static int get_lfo_st3(struct lfo *lfo, int div)
{
	if (lfo->rate == 0 || div == 0)
		return 0;

	/* S3M square */
	if (lfo->type == 2) {
		int val = lfo->phase < WAVEFORM_SIZE / 2 ? 255 : 0;
		return val * lfo->depth / div;
	}

	return get_lfo_mod(lfo, div);
}

/* From OpenMPT VibratoWaveforms.xm:
 * "Generally the vibrato and tremolo tables are identical to those that
 *  ProTracker uses, but the vibrato’s “ramp down” table is upside down."
 */
static int get_lfo_ft2(struct lfo *lfo, int div)
{
	if (lfo->rate == 0 || div == 0)
		return 0;

	/* FT2 ramp */
	if (lfo->type == 1) {
		int phase = (lfo->phase + (WAVEFORM_SIZE >> 1)) % WAVEFORM_SIZE;
		int val = (phase << 3) - 255;
		return val * lfo->depth / div;
	}

	return get_lfo_mod(lfo, div);
}

#ifndef LIBXMP_CORE_DISABLE_IT

static int get_lfo_it(struct lfo *lfo, int div)
{
	if (lfo->rate == 0 || div == 0)
		return 0;

	return get_lfo_st3(lfo, div);
}

#endif

int get_lfo(struct context_data *ctx, struct lfo *lfo, int div, int is_vibrato)
{
	struct module_data *m = &ctx->m;

	switch (m->read_event_type) {
	case READ_EVENT_ST3:
		return get_lfo_st3(lfo, div);
	case READ_EVENT_FT2:
		if (is_vibrato) {
			return get_lfo_ft2(lfo, div);
		} else {
			return get_lfo_mod(lfo, div);
		}
#ifndef LIBXMP_CORE_DISABLE_IT
	case READ_EVENT_IT:
		return get_lfo_it(lfo, div);
#endif
	default:
		return get_lfo_mod(lfo, div);
	}
}


void update_lfo(struct lfo *lfo)
{
	lfo->phase += lfo->rate;
	lfo->phase %= WAVEFORM_SIZE;
}

void set_lfo_phase(struct lfo *lfo, int phase)
{
	lfo->phase = phase;
}

void set_lfo_depth(struct lfo *lfo, int depth)
{
	lfo->depth = depth;
}

void set_lfo_rate(struct lfo *lfo, int rate)
{
	lfo->rate = rate;
}

void set_lfo_waveform(struct lfo *lfo, int type)
{
	lfo->type = type;
}

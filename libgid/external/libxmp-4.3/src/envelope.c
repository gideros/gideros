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

#include "common.h"
#include "envelope.h"

/* Envelope */

int check_envelope_end(struct xmp_envelope *env, int x)
{
	int16 *data = env->data;
	int index;

	if (~env->flg & XMP_ENVELOPE_ON || env->npt <= 0)
		return 0;

	index = (env->npt - 1) * 2;

	/* last node */
	if (x >= data[index] || index == 0) { 
		if (~env->flg & XMP_ENVELOPE_LOOP) {
			return 1;
		}
	}

	return 0;
}

int get_envelope(struct xmp_envelope *env, int x, int def)
{
	int x1, x2, y1, y2;
	int16 *data = env->data;
	int index;

	if (x < 0 || ~env->flg & XMP_ENVELOPE_ON || env->npt <= 0)
		return def;

	index = (env->npt - 1) * 2;

	x1 = data[index];		/* last node */
	if (x >= x1 || index == 0) { 
		return data[index + 1];
	}

	do {
		index -= 2;
		x1 = data[index];
	} while (index > 0 && x1 > x);

	/* interpolate */
	y1 = data[index + 1];
	x2 = data[index + 2];
	y2 = data[index + 3];

	return x2 == x1 ? y2 : ((y2 - y1) * (x - x1) / (x2 - x1)) + y1;
}

static int update_envelope_xm(struct xmp_envelope *env, int x, int release)
{
	int16 *data = env->data;
	int has_loop, has_sus;
	int lpe, lps, sus;

	has_loop = env->flg & XMP_ENVELOPE_LOOP;
	has_sus = env->flg & XMP_ENVELOPE_SUS;

	lps = env->lps << 1;
	lpe = env->lpe << 1;
	sus = env->sus << 1;

	/* FT2 and IT envelopes behave in a different way regarding loops,
	 * sustain and release. When the sustain point is at the end of the
	 * envelope loop end and the key is released, FT2 escapes the loop
	 * while IT runs another iteration. (See EnvLoops.xm in the OpenMPT
	 * test cases.)
	 */
	if (has_loop && has_sus && sus == lpe) {
		if (!release)
			has_sus = 0;
	}

	/* If enabled, stay at the sustain point */
	if (has_sus && !release) {
		if (x >= data[sus]) {
			x = data[sus];
		}
	}

	/* Envelope loops */
	if (has_loop && x >= data[lpe]) {
		if (!(release && has_sus && sus == lpe))
			x = data[lps];
	}

	return x;
}

#ifndef LIBXMP_CORE_DISABLE_IT

static int update_envelope_it(struct xmp_envelope *env, int x, int release, int key_off)
{
	int16 *data = env->data;
	int has_loop, has_sus;
	int lpe, lps, sus, sue;

	has_loop = env->flg & XMP_ENVELOPE_LOOP;
	has_sus = env->flg & XMP_ENVELOPE_SUS;

	lps = env->lps << 1;
	lpe = env->lpe << 1;
	sus = env->sus << 1;
	sue = env->sue << 1;

	/* Release at the end of a sustain loop, run another loop */
	if (has_sus && key_off && x == data[sue] + 1) {
		x = data[sus];
	} else
	/* If enabled, stay in the sustain loop */
	if (has_sus && !release) {
		if (x == data[sue] + 1) {
			x = data[sus];
		}
	} else
	/* Finally, execute the envelope loop */
	if (has_loop) {
		if (x > data[lpe]) {
			x = data[lps];
		}
	}

	return x;
}

#endif

int update_envelope(struct xmp_envelope *env, int x, int release, int key_off, int it_env)
{
	if (x < 0xffff)	{	/* increment tick */
		x++;
	}

	if (x < 0) {
		return -1;
	}

	if (~env->flg & XMP_ENVELOPE_ON || env->npt <= 0) {
		return x;
	}

#ifndef LIBXMP_CORE_DISABLE_IT
	return it_env ?
		update_envelope_it(env, x, release, key_off) :
		update_envelope_xm(env, x, release);
#else
	return update_envelope_xm(env, x, release);
#endif
}


/* Returns: 0 if do nothing, <0 to reset channel, >0 if has fade */
int check_envelope_fade(struct xmp_envelope *env, int x)
{
	int16 *data = env->data;
	int index;

	if (~env->flg & XMP_ENVELOPE_ON)
		return 0;

	index = (env->npt - 1) * 2;		/* last node */
	if (x > data[index]) {
		if (data[index + 1] == 0)
			return -1;
		else
			return 1;
	}

	return 0;
}


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
#include "virtual.h"
#include "mixer.h"
#include "precomp_lut.h"

#ifndef LIBXMP_CORE_PLAYER
#include "synth.h"
#endif

/* Mixers
 *
 * To increase performance eight mixers are defined, one for each
 * combination of the following parameters: interpolation, resolution
 * and number of channels.
 */
#define NEAREST_NEIGHBOR() do { \
    smp_in = sptr[pos]; \
} while (0)

#define LINEAR_INTERP() do { \
    smp_l1 = sptr[pos]; \
    smp_dt = sptr[pos + 1] - smp_l1; \
    smp_in = smp_l1 + (((frac >> 1) * smp_dt) >> (SMIX_SHIFT - 1)); \
} while (0)


/* The following lut settings are PRECOMPUTED. If you plan on changing these
 * settings, you MUST also regenerate the arrays.
 */
/* number of bits used to scale spline coefs */
#define SPLINE_QUANTBITS  14
#define SPLINE_SHIFT    (SPLINE_QUANTBITS)

/* log2(number) of precalculated splines (range is [4..14]) */
#define SPLINE_FRACBITS 10
#define SPLINE_LUTLEN (1L<<SPLINE_FRACBITS)

#define SPLINE_FRACSHIFT ((16 - SPLINE_FRACBITS) - 2)
#define SPLINE_FRACMASK  (((1L << (16 - SPLINE_FRACSHIFT)) - 1) & ~3)

#define SPLINE_INTERP() do { \
    int f = frac >> 6; \
    smp_in = (cubic_spline_lut0[f] * sptr[(int)pos - 1] + \
              cubic_spline_lut1[f] * sptr[pos    ] + \
              cubic_spline_lut3[f] * sptr[pos + 2] + \
              cubic_spline_lut2[f] * sptr[pos + 1]) >> SPLINE_SHIFT; \
} while (0)

#define UPDATE_POS() do { \
    frac += step; \
    pos += frac >> SMIX_SHIFT; \
    frac &= SMIX_MASK; \
} while (0)

#define MIX_STEREO() do { \
    *(buffer++) += smp_in * vr; \
    *(buffer++) += smp_in * vl; \
} while (0)

#define MIX_MONO() do { \
    *(buffer++) += smp_in * vl; \
} while (0)

#define MIX_STEREO_AC() do { \
    if (vi->attack) { \
	int a = SLOW_ATTACK - vi->attack; \
	*(buffer++) += (smp_in * vr * a) >> SLOW_ATTACK_SHIFT; \
	*(buffer++) += (smp_in * vl * a) >> SLOW_ATTACK_SHIFT; \
	vi->attack--; \
    } else { \
	*(buffer++) += smp_in * vr; \
	*(buffer++) += smp_in * vl; \
    } \
} while (0)

#define MIX_STEREO_AC_FILTER() do { \
    sr = (a0 * smp_in * vr + b0 * fr1 + b1 * fr2) >> FILTER_SHIFT; \
    fr2 = fr1; fr1 = sr; \
    sl = (a0 * smp_in * vl + b0 * fl1 + b1 * fl2) >> FILTER_SHIFT; \
    fl2 = fl1; fl1 = sl; \
    if (vi->attack) { \
	int a = SLOW_ATTACK - vi->attack; \
	*(buffer++) += (sr * a) >> SLOW_ATTACK_SHIFT; \
	*(buffer++) += (sl * a) >> SLOW_ATTACK_SHIFT; \
	vi->attack--; \
    } else { \
	*(buffer++) += sr; \
	*(buffer++) += sl; \
    } \
} while (0)

#define MIX_MONO_AC() do { \
    if (vi->attack) { \
	*(buffer++) += (smp_in * vl * (SLOW_ATTACK - vi->attack)) >> SLOW_ATTACK_SHIFT; \
	vi->attack--; \
    } else { \
	*(buffer++) += smp_in * vl; \
    } \
} while (0)

#define MIX_MONO_AC_FILTER() do { \
    sl = (a0 * smp_in * vl + b0 * fl1 + b1 * fl2) >> FILTER_SHIFT; \
    fl2 = fl1; fl1 = sl; \
    if (vi->attack) { \
	*(buffer++) += (sl * (SLOW_ATTACK - vi->attack)) >> SLOW_ATTACK_SHIFT; \
	vi->attack--; \
    } else { \
	*(buffer++) += sl; \
    } \
} while (0)

#define VAR_NORM(x) \
    register int smp_in; \
    x *sptr = vi->sptr; \
    unsigned int pos = vi->pos; \
    int frac = vi->frac

#define VAR_LINEAR(x) \
    VAR_NORM(x); \
    int smp_l1, smp_dt

#define VAR_SPLINE(x) \
    VAR_NORM(x)

#ifndef LIBXMP_CORE_DISABLE_IT

#define VAR_FILTER_MONO \
    int fl1 = vi->filter.l1, fl2 = vi->filter.l2; \
    int64 a0 = vi->filter.a0, b0 = vi->filter.b0, b1 = vi->filter.b1; \
    int sl

#define VAR_FILTER_STEREO \
    VAR_FILTER_MONO; \
    int fr1 = vi->filter.r1, fr2 = vi->filter.r2; \
    int sr

#define SAVE_FILTER_MONO() do { \
    vi->filter.l1 = fl1; \
    vi->filter.l2 = fl2; \
} while (0)

#define SAVE_FILTER_STEREO() do { \
    SAVE_FILTER_MONO(); \
    vi->filter.r1 = fr1; \
    vi->filter.r2 = fr2; \
} while (0)

#endif

#define SMIX_MIXER(f) void f(struct mixer_voice *vi, int *buffer, \
    int count, int vl, int vr, int step)


/*
 * Nearest neighbor mixers
 */

/* Handler for 8 bit samples, linear interpolated stereo output
 */
SMIX_MIXER(smix_stereo_8bit_linear)
{
    VAR_LINEAR(int8);
    while (count--) { LINEAR_INTERP(); MIX_STEREO_AC(); UPDATE_POS(); }
}


/* Handler for 16 bit samples, linear interpolated stereo output
 */
SMIX_MIXER(smix_stereo_16bit_linear)
{
    VAR_LINEAR(int16);

    vl >>= 8;
    vr >>= 8;
    while (count--) { LINEAR_INTERP(); MIX_STEREO_AC(); UPDATE_POS(); }
}


/* Handler for 8 bit samples, nearest neighbor stereo output
 */
SMIX_MIXER(smix_stereo_8bit_nearest)
{
    VAR_NORM(int8);
    while (count--) { NEAREST_NEIGHBOR(); MIX_STEREO(); UPDATE_POS(); }
}


/* Handler for 16 bit samples, nearest neighbor stereo output
 */
SMIX_MIXER(smix_stereo_16bit_nearest)
{
    VAR_NORM(int16);

    vl >>= 8;
    vr >>= 8;
    while (count--) { NEAREST_NEIGHBOR(); MIX_STEREO(); UPDATE_POS(); }
}


/*
 * Linear mixers
 */

/* Handler for 8 bit samples, linear interpolated mono output
 */
SMIX_MIXER(smix_mono_8bit_linear)
{
    VAR_LINEAR(int8);

    while (count--) { LINEAR_INTERP(); MIX_MONO_AC(); UPDATE_POS(); }
}


/* Handler for 16 bit samples, linear interpolated mono output
 */
SMIX_MIXER(smix_mono_16bit_linear)
{
    VAR_LINEAR(int16);

    vl >>= 8;
    while (count--) { LINEAR_INTERP(); MIX_MONO_AC(); UPDATE_POS(); }
}


/* Handler for 8 bit samples, nearest neighbor mono output
 */
SMIX_MIXER(smix_mono_8bit_nearest)
{
    VAR_NORM(int8);

    while (count--) { NEAREST_NEIGHBOR(); MIX_MONO(); UPDATE_POS(); }
}


/* Handler for 16 bit samples, nearest neighbor mono output
 */
SMIX_MIXER(smix_mono_16bit_nearest)
{
    VAR_NORM(int16);

    vl >>= 8;
    while (count--) { NEAREST_NEIGHBOR(); MIX_MONO(); UPDATE_POS(); }
}

#ifndef LIBXMP_CORE_DISABLE_IT

/* Handler for 8 bit samples, linear interpolated stereo output
 */
SMIX_MIXER(smix_stereo_8bit_linear_filter)
{
    VAR_LINEAR(int8);
    VAR_FILTER_STEREO;

    while (count--) { LINEAR_INTERP(); MIX_STEREO_AC_FILTER(); UPDATE_POS(); }
    SAVE_FILTER_STEREO();
}

/* Handler for 16 bit samples, linear interpolated stereo output
 */
SMIX_MIXER(smix_stereo_16bit_linear_filter)
{
    VAR_LINEAR(int16);
    VAR_FILTER_STEREO;

    vl >>= 8;
    vr >>= 8;
    while (count--) { LINEAR_INTERP(); MIX_STEREO_AC_FILTER(); UPDATE_POS(); }
    SAVE_FILTER_STEREO();
}

/* Handler for 8 bit samples, linear interpolated mono output
 */
SMIX_MIXER(smix_mono_8bit_linear_filter)
{
    VAR_LINEAR(int8);
    VAR_FILTER_MONO;

    while (count--) { LINEAR_INTERP(); MIX_MONO_AC_FILTER(); UPDATE_POS(); }
    SAVE_FILTER_MONO();
}

/* Handler for 16 bit samples, linear interpolated mono output
 */
SMIX_MIXER(smix_mono_16bit_linear_filter)
{
    VAR_LINEAR(int16);
    VAR_FILTER_MONO;

    vl >>= 8;
    while (count--) { LINEAR_INTERP(); MIX_MONO_AC_FILTER(); UPDATE_POS(); }
    SAVE_FILTER_MONO();
}

#endif

/*
 * Spline mixers
 */

/* Handler for 8 bit samples, spline interpolated stereo output
 */
SMIX_MIXER(smix_stereo_8bit_spline)
{
    VAR_SPLINE(int8);

    while (count--) { SPLINE_INTERP(); MIX_STEREO_AC(); UPDATE_POS(); }
}


/* Handler for 16 bit samples, spline interpolated stereo output
 */
SMIX_MIXER(smix_stereo_16bit_spline)
{
    VAR_SPLINE(int16);

    vl >>= 8;
    vr >>= 8;
    while (count--) { SPLINE_INTERP(); MIX_STEREO_AC(); UPDATE_POS(); }
}


/* Handler for 8 bit samples, spline interpolated mono output
 */
SMIX_MIXER(smix_mono_8bit_spline)
{
    VAR_SPLINE(int8);

    while (count--) { SPLINE_INTERP(); MIX_MONO_AC(); UPDATE_POS(); }
}


/* Handler for 16 bit samples, spline interpolated mono output
 */
SMIX_MIXER(smix_mono_16bit_spline)
{
    VAR_SPLINE(int16);

    vl >>= 8;
    while (count--) { SPLINE_INTERP(); MIX_MONO_AC(); UPDATE_POS(); }
}

#ifndef LIBXMP_CORE_DISABLE_IT

/* Handler for 8 bit samples, spline interpolated stereo output
 */
SMIX_MIXER(smix_stereo_8bit_spline_filter)
{
    VAR_SPLINE(int8);
    VAR_FILTER_STEREO;

    while (count--) { SPLINE_INTERP(); MIX_STEREO_AC_FILTER(); UPDATE_POS(); }
    SAVE_FILTER_STEREO();
}


/* Handler for 16 bit samples, spline interpolated stereo output
 */
SMIX_MIXER(smix_stereo_16bit_spline_filter)
{
    VAR_SPLINE(int16);
    VAR_FILTER_STEREO;

    vl >>= 8;
    vr >>= 8;
    while (count--) { SPLINE_INTERP(); MIX_STEREO_AC_FILTER(); UPDATE_POS(); }
    SAVE_FILTER_STEREO();
}


/* Handler for 8 bit samples, spline interpolated mono output
 */
SMIX_MIXER(smix_mono_8bit_spline_filter)
{
    VAR_SPLINE(int8);
    VAR_FILTER_MONO;

    while (count--) { SPLINE_INTERP(); MIX_MONO_AC_FILTER(); UPDATE_POS(); }
    SAVE_FILTER_MONO();
}


/* Handler for 16 bit samples, spline interpolated mono output
 */
SMIX_MIXER(smix_mono_16bit_spline_filter)
{
    VAR_SPLINE(int16);
    VAR_FILTER_MONO;

    vl >>= 8;
    while (count--) { SPLINE_INTERP(); MIX_MONO_AC_FILTER(); UPDATE_POS(); }
    SAVE_FILTER_MONO();
}

#endif

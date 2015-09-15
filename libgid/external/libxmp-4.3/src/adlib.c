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
#include "xmp.h"
#include "common.h"
#include "synth.h"
#include "fmopl.h"

/* Use the old GPL-compatible version */
#define USE_OLD_FMOPL

struct adlib {
#ifdef USE_OLD_FMOPL
	FM_OPL *ym3812;
#define YM3812ResetChip(which)	OPLResetChip(a->ym3812)
#define YM3812Write(which,addr,val)	OPLWrite(a->ym3812, addr, val)
#define YM3812Read(which,addr)	OPLRead(a->ym3812, addr)
#define YM3812Init(num,clock,rate) \
		((((struct adlib *)(SYNTH_CHIP(ctx)))->ym3812 = OPLCreate(OPL_TYPE_IO, clock, rate)) != NULL)
#define YM3812Shutdown()	OPLDestroy(a->ym3812)
#define YM3812UpdateOne(which,tmp_bk,count,vl,vr,stereo) \
		YM3812UpdateOne(a->ym3812, tmp_bk, count, vl, vr, stereo)
#endif

#define NUM_SYNTH_CHANNEL 9
	int voc2ch[NUM_SYNTH_CHANNEL];
};

/*
 * ------+-----------------------------------+-----------------------+
 * offset|       SBI data format             |    YM3812 base port
 * ------+-----------------------------------+-----------------------+
 *    0   Modulator Sound Characteristics               20
 *    1   Carrier Sound Characteristics
 *          Bit 7: AM Modulation (am)
 *          Bit 6: Vibrato (vibrato)
 *          Bit 5: Sustaining Sound
 *          Bit 4: Envelop Scaling
 *          Bits 3-0: Frequency Multiplier
 * ------+-----------------------------------+-----------------------+
 *    2   Modulator Scaling/Output Level                40
 *    3   Carrier Scaling/Output Level
 *          Bits 7-6: Level Scaling
 *          Bits 5-0: Output Level
 * ------+-----------------------------------+-----------------------+
 *    4   Modulator Attack/Decay                        60
 *    5   Carrier Attack/Decay
 *          Bits 7-4: Attack Rate
 *          Bits 3-0: Decay Rate
 * ------+-----------------------------------+-----------------------+
 *    6   Modulator Sustain/Release                     80
 *    7   Carrier Sustain/Release
 *          Bits 7-4: Sustain Level
 *          Bits 3-0: Release Rate
 * ------+-----------------------------------+-----------------------+
 *    8   Modulator Wave Select                         E0
 *    9   Carrier Wave Select 
 *          Bits 7-2: All bits clear
 *          Bits 1-0: Wave Select
 * ------+-----------------------------------+-----------------------+
 *   10   Feedback/Connection                           C0
 *          Bits 7-4: All bits clear
 *          Bits 3-1: Modulator Feedback
 *          Bit 0: Connection
 * ------+-----------------------------------+-----------------------+
 */

static const int register_base[11] = {
	0x20, 0x20, 0x40, 0x40,
	0x60, 0x60, 0x80, 0x80,
	0xe0, 0xe0, 0xc0
};

static const int register_offset[2][9] = {
	/* Channel          1     2     3     4     5     6     7     8     9 */
	/* Operator 1 */ {0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12},
	/* Operator 2 */ {0x03, 0x04, 0x05, 0x0B, 0x0C, 0x0D, 0x13, 0x14, 0x15}
};

/*
 *   In octave 4, the F-number values for the chromatic scale and their
 *   corresponding frequencies would be:
 *
 *      F Number     Frequency     Note
 *         16B          277.2       C#
 *         181          293.7       D
 *         198          311.1       D#
 *         1B0          329.6       E
 *         1CA          349.2       F
 *         1E5          370.0       F#
 *         202          392.0       G
 *         220          415.3       G#
 *         241          440.0       A
 *         263          466.2       A#
 *         287          493.9       B
 *         2AE          523.3       C
 */

static const int ym3812_note[] = {
	0x157, 0x16b, 0x181, 0x198, 0x1b0, 0x1ca,
	0x1e5, 0x202, 0x220, 0x241, 0x263, 0x287,
	0x2ae
};


#undef DEBUG_ADLIB

#ifdef DEBUG_ADLIB
#include <sys/io.h>
#include <unistd.h>
#define DELAY(x) do { int i; for (i = 0; i < x; i++) inb (0x388); } while (0)
#endif

/*
 *  After writing to the register port, you must wait twelve cycles before
 *  sending the data; after writing the data, eighty-four cycles must elapse
 *  before any other sound card operation may be performed.
 *
 *  The most accurate method of producing the delay is to read the register
 *  port six times after writing to the register port, and read the register
 *  port thirty-five times after writing to the data port.
 */

static inline int opl_write(struct adlib *a, int addr, int val)
{
#ifdef DEBUG_ADLIB
	outb(addr, 0x388);
	DELAY(5);
	outb(val, 0x389);
	DELAY(35);
	return 0;
#else
	YM3812Write(0, 0, addr);
	return YM3812Write(0, 1, val);
#endif
}

static inline uint8 opl_read(struct adlib *a, int addr)
{
#ifdef DEBUG_ADLIB
	int x;

	outb(addr, 0x388);
	DELAY(5);
	x = inb(0x389);
	DELAY(35);
	return x;
#else
	YM3812Write(0, 0, addr);
	return YM3812Read(0, 1);
#endif
}

static struct adlib *adlib_new()
{
	struct adlib *a;
	int i;

	a = malloc(sizeof (struct adlib));
	if (a == NULL)
		return NULL;

	for (i = 0; i < NUM_SYNTH_CHANNEL; i++) {
		a->voc2ch[i] = -1;
	}

	return a;
}

static void adlib_destroy(struct adlib *a)
{
	free(a);
}

static int synth_getchannel(struct context_data *ctx, int c)
{
	struct adlib *a = SYNTH_CHIP(ctx);
	int i, freech = -1;

	for (c++, i = 0; i < NUM_SYNTH_CHANNEL; i++) {
		if (a->voc2ch[i] == c) {
			return i;
		}

		if (a->voc2ch[i] < 0) {
			freech = i;
			break;
		}
	}
	if (freech != -1) {
		a->voc2ch[freech] = c;
	}

	return freech;
}

static void synth_chreset(struct context_data *ctx)
{
	struct adlib *a = SYNTH_CHIP(ctx);
	int i;

	for (i = 0; i < NUM_SYNTH_CHANNEL; i++) {
		a->voc2ch[i] = -1;
	}
}

static void synth_setpatch(struct context_data *ctx, int c, uint8 *data)
{
	struct adlib *a = SYNTH_CHIP(ctx);
	int i, x;

	if ((c = synth_getchannel(ctx, c)) < 0)
		return;

	for (i = 0; i < 10; i++)
		opl_write(a, register_base[i] + register_offset[i % 2][c],
			  data[i]);
	opl_write(a, register_base[10] + c, data[10]);

	x = opl_read(a, 0xb0 + c);
	opl_write(a, 0xb0 + c, x & ~0x20);
}

/*
 * Bytes A0-B8 - Octave / F-Number / Key-On
 *
 *        7     6     5     4     3     2     1     0
 *     +-----+-----+-----+-----+-----+-----+-----+-----+
 *     |        F-Number (least significant byte)      |  (A0-A8)
 *     |                                               |
 *     +-----+-----+-----+-----+-----+-----+-----+-----+
 *
 *        7     6     5     4     3     2     1     0
 *     +-----+-----+-----+-----+-----+-----+-----+-----+
 *     |  Unused   | Key |    Octave       | F-Number  |  (B0-B8)
 *     |           | On  |                 | most sig. |
 *     +-----+-----+-----+-----+-----+-----+-----+-----+
 *
 *          bit   5  - Channel is voiced when set, silent when clear.
 *          bits 4-2 - Octave (0-7).  0 is lowest, 7 is highest.
 *          bits 1-0 - Most significant bits of F-number.
 */

static void synth_setnote(struct context_data *ctx, int c, int note, int bend)
{
	struct adlib *a = SYNTH_CHIP(ctx);
	int n, f, o;

	if ((c = synth_getchannel(ctx, c)) < 0)
		return;

	n = note % 12;
	f = ym3812_note[n] + (ym3812_note[n + 1] - ym3812_note[n]) * bend / 100;
	o = note / 12 - 2;

	if (o < 0)
		o = 0;

	opl_write(a, 0xa0 + c, f & 0xff);
	opl_write(a, 0xb0 + c, 0x20 | ((o << 2) & 0x1c) | ((f >> 8) & 0x03));
}

static void synth_setvol(struct context_data *ctx, int c, int vol)
{
	struct adlib *a = SYNTH_CHIP(ctx);
	int b, ofs;

	if ((c = synth_getchannel(ctx, c)) < 0)
		return;

	if (vol > 63)
		vol = 63;

	/* Check if operator 1 produces sound */
	if (opl_read(a, 0xc8 + c)) {
		ofs = register_offset[0][c];
		b = opl_read(a, 0x40 + ofs);
		opl_write(a, 0x40 + ofs, (b & 0xc0) | (63 - vol));
	}

	ofs = register_offset[1][c];
	b = opl_read(a, 0x40 + ofs);
	opl_write(a, 0x40 + ofs, (b & 0xc0) | (63 - vol));
}

static int synth_init(struct context_data *ctx, int freq)
{
	SYNTH_CHIP(ctx) = adlib_new();
	if (SYNTH_CHIP(ctx) == NULL)
		return -1;

#ifdef DEBUG_ADLIB
	ioperm(0x388, 2, 1);
#endif
	synth_chreset(ctx);

	return YM3812Init(1, 3579545, freq);
}

static int synth_reset(struct context_data *ctx)
{
	struct adlib *a = SYNTH_CHIP(ctx);

#ifdef DEBUG_ADLIB
	int i;

	for (i = 0; i < 9; i++) {
		opl_write(a, ym3812, 0xb0 + i, 0);
	}
#else
	YM3812ResetChip(0);
#endif
	synth_chreset(ctx);

	return 0;
}

static int synth_deinit(struct context_data *ctx)
{
	struct adlib *a = SYNTH_CHIP(ctx);

	synth_reset(ctx);
	YM3812Shutdown();

	adlib_destroy(a);

	return 0;
}

static void synth_mixer(struct context_data *ctx, int32 *tmp_bk, int count, int vl, int vr, int stereo)
{
	struct adlib *a = SYNTH_CHIP(ctx);

	if (!tmp_bk)
		return;

	YM3812UpdateOne(0, tmp_bk, count, vl, vr, stereo);
}


const struct synth_info synth_adlib = {
	synth_init,
	synth_deinit,
	synth_reset,
	synth_setpatch,
	synth_setnote,
	synth_setvol,
	synth_mixer
};

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

#include "stdio2.h"
#include <string.h>
#include "common.h"
#include "period.h"

#include <math.h>

/* Amiga periods */
static const int period_amiga[] = {
   /*  0       1       2       3       4       5       6       7   */
    0x1c56, 0x1c22, 0x1bee, 0x1bbb, 0x1b87, 0x1b55, 0x1b22, 0x1af0,  /* B  */
    0x1abf, 0x1a8e, 0x1a5d, 0x1a2c, 0x19fc, 0x19cc, 0x199c, 0x196d,  /* C  */
    0x193e, 0x1910, 0x18e2, 0x18b4, 0x1886, 0x1859, 0x182c, 0x1800,  /* C# */
    0x17d4, 0x17a8, 0x177c, 0x1751, 0x1726, 0x16fb, 0x16d1, 0x16a7,  /* D  */
    0x167d, 0x1654, 0x162b, 0x1602, 0x15d9, 0x15b1, 0x1589, 0x1562,  /* D# */
    0x153a, 0x1513, 0x14ec, 0x14c6, 0x149f, 0x1479, 0x1454, 0x142e,  /* E  */
    0x1409, 0x13e4, 0x13c0, 0x139b, 0x1377, 0x1353, 0x1330, 0x130c,  /* F  */
    0x12e9, 0x12c6, 0x12a4, 0x1282, 0x125f, 0x123e, 0x121c, 0x11fb,  /* F# */
    0x11da, 0x11b9, 0x1198, 0x1178, 0x1157, 0x1137, 0x1118, 0x10f8,  /* G  */
    0x10d9, 0x10ba, 0x109b, 0x107d, 0x105e, 0x1040, 0x1022, 0x1004,  /* G# */
    0x0fe7, 0x0fca, 0x0fad, 0x0f90, 0x0f73, 0x0f57, 0x0f3a, 0x0f1e,  /* A  */
    0x0f02, 0x0ee7, 0x0ecb, 0x0eb0, 0x0e95, 0x0e7a, 0x0e5f, 0x0e45,  /* A# */
    0x0e2b, 0x0e11, 0x0df7, 0x0ddd, 0x0dc3, 0x0daa, 0x0d91, 0x0d78,  /* B  */
};

#ifdef _MSC_VER
static inline double round(double val)
{    
	return floor(val + 0.5);
}
#endif

/* Get period from note */
inline double note_to_period(int n, int f, int type, double adj)
{
    double d = (double)n + (double)f / 128;
    double per;

    per = type ?
	(240.0 - d) * 16 :			/* Linear */
        13694.0 / pow(2, d / 12);		/* Amiga */

#ifndef LIBXMP_CORE_PLAYER
    if (adj > 0.1)
	per *= adj;
#endif

    return per;
}


/* For the software mixer */
int note_to_period_mix(int n, int b)
{
    double d = (double)n + (double)b / 12800;
    return (int)(8192.0 * XMP_PERIOD_BASE / pow(2, d / 12));
}


/* Get note from period using the Amiga frequency table */
/* This function is used only by the MOD loader */
int period_to_note(int p)
{
    int n, f;
    const int *t = period_amiga + MAX_NOTE;

    if (!p)
	return 0;
    for (n = 12 + NOTE_Bb0; p <= (MAX_PERIOD / 2); n += 12, p <<= 1);
    for (; p > *t; t -= 8, n--);
    for (f = 7; f && (*t > p); t++, f--);
    return n - (f >> 2);
}


/* Get pitchbend from base note and amiga period */
int period_to_bend(double p, int n, int gliss, int type, double adj)
{
    int b;
    double d;

    if (n == 0)
	return 0;

    if (type) {
    	b = 100 * (8 * (((240 - n) << 4) - p));	/* Linear */
    	return gliss ? b / 12800 * 12800 : b;
    }

    d = note_to_period(n, 0, 0, adj);
    b = round(100.0 * ((1536.0 * log(d / p) / M_LN2)));

    return gliss ? b / 12800 * 12800 : b;	/* Amiga */
}


/* Convert finetune = 1200 * log2(C2SPD/8363))
 *
 *      c = (1200.0 * log(c2spd) - 1200.0 * log(c4_rate)) / M_LN2;
 *      xpo = c/100;
 *      fin = 128 * (c%100) / 100;
 */
void c2spd_to_note(int c2spd, int *n, int *f)
{
    int c;

    if (c2spd == 0) {
	*n = *f = 0;
	return;
    }

    c = (int)(1536.0 * log((double)c2spd / 8363) / M_LN2);
    *n = c / 128;
    *f = c % 128;
}

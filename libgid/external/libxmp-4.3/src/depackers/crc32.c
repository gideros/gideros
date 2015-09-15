/*
 * CRC functions for libxmp
 * Copyright (C) 2013 Claudio Matsuoka
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
#include "crc32.h"

uint32 crc32_table_A[256];
uint32 crc32_table_B[256];

static void crc_table_init_A(uint32 poly, uint32 *table)
{
	int i, j;
	uint32 k;

	for (i = 0; i < 256; i++) {
		k = i;
		for (j = 0; j < 8; j++) {
			k = k & 1 ? (k >> 1) ^ poly : k >> 1;
		}

		table[i] = k;
	}

	return;
}

static void crc_table_init_B(uint32 poly, uint32 *table)
{
	int i, j;
	uint32 k;

	for (i = 0; i < 256; i++) {
		k = i << 24;
		for (j = 0; j < 8; j++) {
			k = k & 0x80000000 ? (k << 1) ^ poly : k << 1;
		}

		table[i] = k;
	}

	return;
}

void crc32_init_A()
{
	static int flag = 0;

	if (flag)
		return;

	crc_table_init_A(0xedb88320, crc32_table_A);

	flag = 1;
}

void crc32_init_B()
{
	static int flag = 0;

	if (flag)
		return;

	crc_table_init_B(0x04c11db7, crc32_table_B);

	flag = 1;
}

uint32 crc32_A1(const uint8 *buf, size_t size, uint32 crc)
{
	crc = ~crc;

        while (size--) {
                crc = crc32_table_A[*buf++ ^ (crc & 0xff)] ^ (crc >> 8);
        }

        return ~crc;
}

uint32 crc32_A2(const uint8 *buf, size_t size, uint32 crc)
{
        while (size--) {
                crc = crc32_table_A[*buf++ ^ (crc & 0xff)] ^ (crc >> 8);
        }

        return crc;
}

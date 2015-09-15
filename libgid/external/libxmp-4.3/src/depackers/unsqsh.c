/*
 * XPK-SQSH depacker
 * Algorithm from the portable decruncher by Bert Jahn (24.12.97)
 * Checksum added by Sipos Attila <h430827@stud.u-szeged.hu>
 * Rewritten for libxmp by Claudio Matsuoka
 *
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

#include "stdio2.h"
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "depacker.h"

struct io {
	uint8 *src;
	uint8 *dest;
	int offs;
};

static uint8 ctable[] = {
	2, 3, 4, 5, 6, 7, 8, 0,
	3, 2, 4, 5, 6, 7, 8, 0,
	4, 3, 5, 2, 6, 7, 8, 0,
	5, 4, 6, 2, 3, 7, 8, 0,
	6, 5, 7, 2, 3, 4, 8, 0,
	7, 6, 8, 2, 3, 4, 5, 0,
	8, 7, 6, 2, 3, 4, 5, 0
};

static uint16 xchecksum(uint32 * ptr, uint32 count)
{
	register uint32 sum = 0;

	while (count-- > 0) {
		sum ^= *ptr++;
	}

	return (uint16) (sum ^ (sum >> 16));
}

static int get_bits(struct io *io, int count)
{
	int r = readmem24b(io->src + (io->offs >> 3));

	r <<= io->offs % 8;
	r &= 0xffffff;
	r >>= 24 - count;
	io->offs += count;

	return r;
}

static int get_bits_final(struct io *io, int count)
{
	int r = readmem24b(io->src + (io->offs >> 3));

	r <<= (io->offs % 8) + 8;
	r >>= 32 - count;
	io->offs += count;

	return r;
}

static int copy_data(struct io *io, int d1, int *data, uint8 *dest_start, uint8 *dest_end)
{
	uint8 *copy_src;
	int dest_offset, count, copy_len;

	if (get_bits(io, 1) == 0) {
		copy_len = get_bits(io, 1) + 2;
	} else if (get_bits(io, 1) == 0) {
		copy_len = get_bits(io, 1) + 4;
	} else if (get_bits(io, 1) == 0) {
		copy_len = get_bits(io, 1) + 6;
	} else if (get_bits(io, 1) == 0) {
		copy_len = get_bits(io, 3) + 8;
	} else {
		copy_len = get_bits(io, 5) + 16;
	}

	if (get_bits(io, 1) == 0) {
		if (get_bits(io, 1) == 0) {
			count = 8;
			dest_offset = 0;
		} else {
			count = 14;
			dest_offset = -0x1100;
		}
	} else {
		count = 12;
		dest_offset = -0x100;
	}

	copy_len -= 3;

	if (copy_len >= 0) {
		if (copy_len != 0) {
			d1--;
		}
		d1--;
		if (d1 < 0) {
			d1 = 0;
		}
	}

	copy_len += 2;

	copy_src = io->dest + dest_offset - get_bits(io, count) - 1;

	/* Sanity check */
	if (copy_src < dest_start || copy_src + copy_len >= dest_end) {
		return -1;
	}

	do {
		//printf("dest=%p src=%p end=%p\n", io->dest, copy_src, dest_end);
		*io->dest++ = *copy_src++;
	} while (copy_len--);

	*data = *(--copy_src);

	return d1;
}

static int unsqsh_block(struct io *io, uint8 *dest_start, uint8 *dest_end)
{
	int d1, d2, data, unpack_len, count, old_count;

	d1 = d2 = data = old_count = 0;
	io->offs = 0;

	data = *(io->src++);
	*(io->dest++) = data;

	do {
		if (d1 < 8) {
			if (get_bits(io, 1)) {
				d1 = copy_data(io, d1, &data, dest_start, dest_end);
				if (d1 < 0)
					return -1;
				d2 -= d2 >> 3;
				continue;
			} 
			unpack_len = 0;
			count = 8;
		} else {
			if (get_bits(io, 1)) {
				count = 8;
				if (count == old_count) {
					if (d2 >= 20) {
						unpack_len = 1;
						d2 += 8;
					} else {
						unpack_len = 0;
					}
				} else {
					count = old_count;
					unpack_len = 4;
					d2 += 8;
				}
			} else {
				if (get_bits(io, 1) == 0) {
					d1 = copy_data(io, d1, &data, dest_start, dest_end);
					if (d1 < 0)
						return -1;
	      				d2 -= d2 >> 3;
					continue;
				}

				if (get_bits(io, 1) == 0) {
					count = 2;
				} else {
					if (get_bits(io, 1)) {
						io->offs--;
						count = get_bits(io, 3);
					} else {
						count = 3;
					}
				}

				count = ctable[8 * old_count + count - 17];
				if (count != 8) {
					unpack_len = 4;
					d2 += 8;
				} else {
					if (d2 >= 20) {
						unpack_len = 1;
						d2 += 8;
					} else {
						unpack_len = 0;
					}
				}
			}
		}

		do {
			data -= get_bits_final(io, count);
			*io->dest++ = data;
		} while (unpack_len--);

		if (d1 != 31) {
			d1++;
		}

		old_count = count;

		d2 -= d2 >> 3;

	} while (io->dest < dest_end);

	return 0;
}

static int unsqsh(uint8 *src, int srclen, uint8 *dest, int destlen)
{
	int len = destlen;
	int decrunched = 0;
	int type;
	int sum, packed_size, unpacked_size;
	int lchk;
	uint8 *c, *dest_start, *dest_end;
	uint8 bc[3];
	struct io io;

	io.src = src;
	io.dest = dest;

	dest_start = io.dest;

	c = src + 20;

	while (len) {
		/* Sanity check */
		if (c >= src + srclen) {
			return -1;
		}

		type = *c++;
		c++;			/* hchk */

		sum = *(uint16 *)c;
		c += 2;			/* checksum */

		packed_size = readmem16b(c);	/* packed */
		c += 2;

		unpacked_size = readmem16b(c);	/* unpacked */
		c += 2;

		/* Sanity check */
		if (packed_size <= 0 || unpacked_size <= 0) {
			return -1;
		}

		if (c + packed_size + 3 > src + srclen) {
			return -1;
		}

		io.src = c + 2;
		memcpy(bc, c + packed_size, 3);
		memset(c + packed_size, 0, 3);
		lchk = xchecksum((uint32 *) (c), (packed_size + 3) >> 2);
		memcpy(c + packed_size, bc, 3);

		if (lchk != sum) {
			return decrunched;
		}

		if (type == 0) {
			/* verbatim block */
			memcpy(io.dest, c, packed_size);
			io.dest += packed_size;
			c += packed_size;
			len -= packed_size;
			decrunched += packed_size;
			continue;
		}

		if (type != 1) {
			/* unknown type */
			return decrunched;
		}

		len -= unpacked_size;
		decrunched += unpacked_size;

		/* Sanity check */
		if (decrunched > destlen) {
			return -1;
		}

		packed_size = (packed_size + 3) & 0xfffc;
		c += packed_size;
		dest_end = io.dest + unpacked_size;

		if (unsqsh_block(&io, dest_start, dest_end) < 0) {
			return -1;
		}
		
		io.dest = dest_end;
	}

	return decrunched;

}

static int test_sqsh(unsigned char *b)
{
	return memcmp(b, "XPKF", 4) == 0 && memcmp(b + 8, "SQSH", 4) == 0;
}

static int decrunch_sqsh(FILE * f, FILE * fo)
{
	unsigned char *src, *dest;
	int srclen, destlen;

	if (read32b(f) != 0x58504b46)	/* XPKF */
		goto err;

	srclen = read32b(f);

	/* Sanity check */
	if (srclen <= 8 || srclen > 0x100000)
		goto err;

	if (read32b(f) != 0x53515348)	/* SQSH */
		goto err;

	destlen = read32b(f);
	if (destlen < 0 || destlen > 0x100000)
		goto err;

	if ((src = malloc(srclen + 3)) == NULL)
		goto err;

	if ((dest = malloc(destlen + 100)) == NULL)
		goto err2;

	if (fread(src, srclen - 8, 1, f) != 1)
		goto err3;

	if (unsqsh(src, srclen, dest, destlen) != destlen)
		goto err3;

	if (fwrite(dest, destlen, 1, fo) != 1)
		goto err3;

	free(dest);
	free(src);

	return 0;

    err3:
	free(dest);
    err2:
	free(src);
    err:
	return -1;
}

struct depacker sqsh_depacker = {
	test_sqsh,
	decrunch_sqsh
};

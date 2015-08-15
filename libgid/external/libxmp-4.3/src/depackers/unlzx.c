/*  $Id: LZX.c,v 1.12 2005/06/23 14:54:41 stoecker Exp $
    LZX file archiver client

    XAD library system for archive handling
    Copyright (C) 1998 and later by Dirk Stöcker <soft@dstoecker.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
    Modified for xmp by Claudio Matsuoka, 2014-01-04
*/

#include "stdio2.h"
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "depacker.h"
#include "crc32.h"

#if 0
#ifndef XADMASTERVERSION
#define XADMASTERVERSION      10
#endif

XADCLIENTVERSTR("LZX 1.10 (21.2.2004)")
#define LZX_VERSION             1
#define LZX_REVISION            10
/* ---------------------------------------------------------------------- */
#define LZXINFO_DAMAGE_PROTECT 1
#define LZXINFO_FLAG_LOCKED 2
struct LZXInfo_Header {
    uint8 ID[3];	/* "LZX" */
    uint8 Flags;	/* LZXINFO_FLAG_#? */
    uint8 Unknown[6];
};
#endif

#define LZXHDR_FLAG_MERGED      (1<<0)

#define LZXHDR_PROT_READ        (1<<0)
#define LZXHDR_PROT_WRITE       (1<<1)
#define LZXHDR_PROT_DELETE      (1<<2)
#define LZXHDR_PROT_EXECUTE     (1<<3)
#define LZXHDR_PROT_ARCHIVE     (1<<4)
#define LZXHDR_PROT_HOLD        (1<<5)
#define LZXHDR_PROT_SCRIPT      (1<<6)
#define LZXHDR_PROT_PURE        (1<<7)

#define LZXHDR_TYPE_MSDOS       0
#define LZXHDR_TYPE_WINDOWS     1
#define LZXHDR_TYPE_OS2         2
#define LZXHDR_TYPE_AMIGA       10
#define LZXHDR_TYPE_UNIX        20

#define LZXHDR_PACK_STORE       0
#define LZXHDR_PACK_NORMAL      2
#define LZXHDR_PACK_EOF         32

struct LZXArc_Header {
    uint8 Attributes;	/*  0 - LZXHDR_PROT_#? */
    uint8 pad1;		/*  1 */
    uint8 FileSize[4];	/*  2 (little endian) */
    uint8 CrSize[4];	/*  6 (little endian) */
    uint8 MachineType;	/* 10 - LZXHDR_TYPE_#? */
    uint8 PackMode;	/* 11 - LZXHDR_PACK_#? */
    uint8 Flags;	/* 12 - LZXHDR_FLAG_#? */
    uint8 pad2;		/* 13 */
    uint8 CommentSize;	/* 14 - length (0-79) */
    uint8 ExtractVersion;	/* 15 - version needed to extract */
    uint8 pad3;		/* 16 */
    uint8 pad4;		/* 17 */
    uint8 Date[4];	/* 18 - Packed_Date */
    uint8 DataCRC[4];	/* 22 (little endian) */
    uint8 HeaderCRC[4];	/* 26 (little endian) */
    uint8 FilenameSize;	/* 30 - filename length */
};				/* SIZE = 31 */

/* Header CRC includes filename and comment. */

#define LZXHEADERSIZE   31

/* Packed date [4 BYTES, bit 0 is MSB, 31 is LSB]
  bit  0 -  4   Day
       5 -  8   Month   (January is 0)
       9 - 14   Year    (start 1970)
      15 - 19   Hour
      20 - 25   Minute
      26 - 31   Second
*/

struct LZXEntryData {
    uint32 CRC;		/* CRC of uncrunched data */
    uint32 PackMode;	/* CrunchMode */
    uint32 ArchivePos;	/* Position is src file */
    uint32 DataStart;	/* Position in merged buffer */
};

#define LZXPE(a)	((struct LZXEntryData *) ((a)->xfi_PrivateInfo))
#define LZXDD(a)	((struct LZXDecrData *) ((a)->xai_PrivateClient))
struct LZXDecrData {
    int mode;

    uint8 archive_header[31];
    uint8 header_filename[256];
    uint8 header_comment[256];
    uint32 crc;
    uint8 pack_mode;
    uint32 sum;
    FILE *outfile;

    struct filename_node *filename_list;

    uint8 *src;
    uint8 *dest;
    uint8 *src_end;
    uint8 *dest_end;

    uint32 method;
    uint32 decrunch_length;
    uint32 pack_size;
    uint32 unpack_size;
    uint32 last_offset;
    uint32 control;
    int shift;

    uint8 offset_len[8];
    uint16 offset_table[128];
    uint8 huffman20_len[20];
    uint16 huffman20_table[96];
    uint8 literal_len[768];
    uint16 literal_table[5120];

    uint8 read_buffer[16384];	/* have a reasonable sized read buffer */
    uint8 buffer[258 + 65536 + 258];	/* allow overrun for speed */
};

/* ---------------------------------------------------------------------- */
#if 0
XADRECOGDATA(LZX)
{
    if (data[0] == 'L' && data[1] == 'Z' && data[2] == 'X')
	return 1;
    else
	return 0;
}

#define XADFIBF_DELETE  (1<<0)
#define XADFIBF_EXECUTE (1<<1)
#define XADFIBF_WRITE   (1<<2)
#define XADFIBF_READ    (1<<3)
#define XADFIBF_PURE    (1<<4)

XADGETINFO(LZX)
{
    int err;
    uint32 bufpos = 0;
    struct xadFileInfo *fi, *fig = 0;	/* fig - first grouped ptr */
    struct LZXArc_Header head;

    if (!
	(err =
	 xadHookAccess(XADM XADAC_INPUTSEEK, sizeof(struct LZXInfo_Header), 0,
		       ai))) {
	while (!err && ai->xai_InPos < ai->xai_InSize) {
	    if (!
		(err =
		 xadHookAccess(XADM XADAC_READ, LZXHEADERSIZE, &head, ai))) {
		uint32 i, j, k, l, crc;
		i = head.CommentSize;
		j = head.FilenameSize;
		k = EndGetI32(head.HeaderCRC);
		head.HeaderCRC[0] = head.HeaderCRC[1] = head.HeaderCRC[2] =
		    head.HeaderCRC[3] = 0;
		/* clear for CRC check */

		if (!
		    (fi =
		     (struct xadFileInfo *)xadAllocObject(XADM XADOBJ_FILEINFO,
							  XAD_OBJNAMESIZE,
							  j + 1,
							  i ? XAD_OBJCOMMENTSIZE
							  : TAG_IGNORE, i + 1,
							  XAD_OBJPRIVINFOSIZE,
							  sizeof(struct
								 LZXEntryData),
							  TAG_DONE)))
		    err = XADERR_NOMEMORY;
		else if (!
			 (err =
			  xadHookAccess(XADM XADAC_READ, j, fi->xfi_FileName,
					ai)) && (!i
						 || !(err =
						      xadHookAccess(XADM
								    XADAC_READ,
								    i,
								    fi->
								    xfi_Comment,
								    ai)))) {
		    l = EndGetI32(head.CrSize);

		    if (!l
			|| !(err =
			     xadHookAccess(XADM XADAC_INPUTSEEK, l, 0, ai))) {
			crc =
			    xadCalcCRC32(XADM XADCRC32_ID1, (uint32)~0,
					 LZXHEADERSIZE, (uint8 *)&head);
			crc =
			    xadCalcCRC32(XADM XADCRC32_ID1, crc, j,
					 (uint8 *)fi->xfi_FileName);
			if (i)
			    crc =
				xadCalcCRC32(XADM XADCRC32_ID1, crc, i,
					     (uint8 *)fi->xfi_Comment);

			if (~crc != k)
			    err = XADERR_CHECKSUM;
			else {
			    if (!fig) {
				fig = fi;
				bufpos = 0;
			    }
			    fi->xfi_Size = EndGetI32(head.FileSize);
			    if (!l && !fi->xfi_Size
				&& fi->xfi_FileName[--j] == '/') {
				fi->xfi_FileName[j] = 0;
				fi->xfi_Flags |= XADFIF_DIRECTORY;
			    }

			    i = head.Attributes;
			    j = 0;

			    if (!(i & LZXHDR_PROT_READ))
				j |= XADFIBF_READ;
			    if (!(i & LZXHDR_PROT_WRITE))
				j |= XADFIBF_WRITE;
			    if (!(i & LZXHDR_PROT_DELETE))
				j |= XADFIBF_DELETE;
			    if (!(i & LZXHDR_PROT_EXECUTE))
				j |= XADFIBF_EXECUTE;
			    j |= (i &
				  (LZXHDR_PROT_ARCHIVE | LZXHDR_PROT_SCRIPT));
			    if (i & LZXHDR_PROT_PURE)
				j |= XADFIBF_PURE;
			    if (i & LZXHDR_PROT_HOLD)
				j |= (1 << 7);	/* not defined in <dos/dos.h> */
			    fi->xfi_Protection = j;

			    {	/* Make the date */
				struct xadDate d;
				j = EndGetM32(head.Date);
				d.xd_Second = j & 63;
				j >>= 6;
				d.xd_Minute = j & 63;
				j >>= 6;
				d.xd_Hour = j & 31;
				j >>= 5;
				d.xd_Year = 1970 + (j & 63);
				if (d.xd_Year >= 2028)	/* Original LZX */
				    d.xd_Year += 2000 - 2028;
				else if (d.xd_Year < 1978)	/* Dr.Titus */
				    d.xd_Year += 2034 - 1970;
				/* Dates from 1978 to 1999 are correct */
				/* Dates from 2000 to 2027 Mikolaj patch are correct */
				/* Dates from 2000 to 2005 LZX/Dr.Titus patch are correct */
				/* Dates from 2034 to 2041 Dr.Titus patch are correct */
				j >>= 6;
				d.xd_Month = 1 + (j & 15);
				j >>= 4;
				d.xd_Day = j;
				d.xd_Micros = 0;
				xadConvertDates(XADM XAD_DATEXADDATE, &d,
						XAD_GETDATEXADDATE,
						&fi->xfi_Date, TAG_DONE);
			    }
			    LZXPE(fi)->CRC = EndGetI32(head.DataCRC);
			    LZXPE(fi)->DataStart = bufpos;
			    bufpos += fi->xfi_Size;
			    if (head.Flags & LZXHDR_FLAG_MERGED) {
				fi->xfi_Flags |= XADFIF_GROUPED;
				if (l) {
				    fi->xfi_Flags |= XADFIF_ENDOFGROUP;
				    fi->xfi_GroupCrSize = l;
				}
			    } else
				fi->xfi_CrunchSize = l;

			    if (l) {
				LZXPE(fi)->ArchivePos = ai->xai_InPos - l;
				LZXPE(fi)->PackMode = head.PackMode;
				while (fig) {
				    fig->xfi_GroupCrSize = l;
				    LZXPE(fig)->ArchivePos = ai->xai_InPos - l;
				    LZXPE(fig)->PackMode = head.PackMode;
				    fig = fig->xfi_Next;
				}
			    }

			    err = xadAddFileEntryA(XADM fi, ai, 0);
			    fi = 0;
			}	/* skip crunched data */
		    }		/* get filename and comment */
		    if (fi)
			xadFreeObjectA(XADM fi, 0);
		}		/* xadFileInfo Allocation */
	    }			/* READ header */
	}			/* while loop */
    }
    /* INPUTSEEK 3 bytes */
    if (err && ai->xai_FileInfo) {
	ai->xai_Flags |= XADAIF_FILECORRUPT;
	ai->xai_LastError = err;
	err = 0;
    }

    return err;
}
#endif

struct filename_node {
    struct filename_node *next;
    uint32 length;
    uint32 crc;
    char filename[256];
};

/* ---------------------------------------------------------------------- */

static const uint8 table_one[32] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10,
    11, 11, 12, 12, 13, 13, 14, 14
};

static const uint32 table_two[32] = {
    0, 1, 2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64, 96, 128, 192, 256, 384, 512,
    768, 1024,
    1536, 2048, 3072, 4096, 6144, 8192, 12288, 16384, 24576, 32768, 49152
};

static const uint32 table_three[16] = {
    0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767
};

static const uint8 table_four[34] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16
};

/* ---------------------------------------------------------------------- */

/* Build a fast huffman decode table from the symbol bit lengths.         */
/* There is an alternate algorithm which is faster but also more complex. */

static int make_decode_table(int number_symbols, int table_size,
			     uint8 *length, uint16 *table)
{
    register uint8 bit_num = 0;
    register int symbol;
    uint32 leaf;		/* could be a register */
    uint32 table_mask, bit_mask, pos, fill, next_symbol, reverse;

    pos = 0;			/* current position in decode table */

    bit_mask = table_mask = 1 << table_size;

    bit_mask >>= 1;		/* don't do the first number */
    bit_num++;

    while (bit_num <= table_size) {
	for (symbol = 0; symbol < number_symbols; symbol++) {
	    if (length[symbol] == bit_num) {
		reverse = pos;	/* reverse the order of the position's bits */
		leaf = 0;
		fill = table_size;

		do {		/* reverse the position */
		    leaf = (leaf << 1) + (reverse & 1);
		    reverse >>= 1;
		} while (--fill);

		if ((pos += bit_mask) > table_mask)
		    return -1;	/* we will overrun the table! abort! */

		fill = bit_mask;
		next_symbol = 1 << bit_num;

		do {
		    table[leaf] = symbol;
		    leaf += next_symbol;
		} while (--fill);
	    }
	}
	bit_mask >>= 1;
	bit_num++;
    }

    if (pos != table_mask) {
	for (symbol = pos; symbol < table_mask; symbol++) {	/* clear the rest of the table */
	    reverse = symbol;	/* reverse the order of the position's bits */
	    leaf = 0;
	    fill = table_size;

	    do {		/* reverse the position */
		leaf = (leaf << 1) + (reverse & 1);
		reverse >>= 1;
	    } while (--fill);

	    table[leaf] = 0;
	}

	next_symbol = table_mask >> 1;
	pos <<= 16;
	table_mask <<= 16;
	bit_mask = 32768;

	while (bit_num <= 16) {
	    for (symbol = 0; symbol < number_symbols; symbol++) {
		if (length[symbol] == bit_num) {
		    reverse = pos >> 16;	/* reverse the order of the position's bits */
		    leaf = 0;
		    fill = table_size;

		    do {	/* reverse the position */
			leaf = (leaf << 1) + (reverse & 1);
			reverse >>= 1;
		    } while (--fill);

		    for (fill = 0; fill < bit_num - table_size; fill++) {
			if (table[leaf] == 0) {
			    table[next_symbol << 1] = 0;
			    table[(next_symbol << 1) + 1] = 0;
			    table[leaf] = next_symbol++;
			}
			leaf = table[leaf] << 1;
			leaf += (pos >> (15 - fill)) & 1;
		    }

		    table[leaf] = symbol;
		    pos += bit_mask;

		    if (pos > table_mask)
			return -1;	/* we will overrun the table! abort! */
		}
	    }
	    bit_mask >>= 1;
	    bit_num++;
	}
    }

    if (pos != table_mask)
	return -1;		/* the table is incomplete! */

    return 0;
}

/* ---------------------------------------------------------------------- */
/* Read and build the decrunch tables. There better be enough data in the */
/* src buffer or it's stuffed. */

static int read_literal_table(struct LZXDecrData *decr)
{
    register uint32 control;
    register int shift;
    uint32 temp;		/* could be a register */
    uint32 symbol, pos, count, fix, max_symbol;
    uint8 *src;
    int abort = 0;
    int x;

    control = decr->control;
    shift = decr->shift;
    src = decr->src;

    if (shift < 0) {		/* fix the control word if necessary */
	shift += 16;
	control += *src++ << (8 + shift);
	control += *src++ << shift;
    }

    /* read the decrunch method */

    decr->method = control & 7;
    control >>= 3;
    shift -= 3;

    if (shift < 0) {
	shift += 16;
	control += *src++ << (8 + shift);
	control += *src++ << shift;
    }

    /* Read and build the offset huffman table */

    if (!abort && decr->method == 3) {
	for (temp = 0; temp < 8; temp++) {
	    decr->offset_len[temp] = control & 7;
	    control >>= 3;
	    if ((shift -= 3) < 0) {
		shift += 16;
		control += *src++ << (8 + shift);
		control += *src++ << shift;
	    }
	}
	abort = make_decode_table(8, 7, decr->offset_len, decr->offset_table);
    }

    /* read decrunch length */

    if (!abort) {
	decr->decrunch_length = (control & 255) << 16;
	control >>= 8;
	shift -= 8;

	if (shift < 0) {
	    shift += 16;
	    control += *src++ << (8 + shift);
	    control += *src++ << shift;
	}

	decr->decrunch_length += (control & 255) << 8;
	control >>= 8;
	shift -= 8;

	if (shift < 0) {
	    shift += 16;
	    control += *src++ << (8 + shift);
	    control += *src++ << shift;
	}

	decr->decrunch_length += (control & 255);
	control >>= 8;
	shift -= 8;

	if (shift < 0) {
	    shift += 16;
	    control += *src++ << (8 + shift);
	    control += *src++ << shift;
	}
    }

    /* read and build the huffman literal table */

    if (!abort && decr->method != 1) {
	pos = 0;
	fix = 1;
	max_symbol = 256;

	do {
	    for (temp = 0; temp < 20; temp++) {
		decr->huffman20_len[temp] = control & 15;
		control >>= 4;
		if ((shift -= 4) < 0) {
		    shift += 16;
		    control += *src++ << (8 + shift);
		    control += *src++ << shift;
		}
	    }
	    abort = make_decode_table(20, 6, decr->huffman20_len,
				  decr->huffman20_table);

	    if (abort)
		break;		/* argh! table is corrupt! */

	    do {
		if ((symbol = decr->huffman20_table[control & 63]) >= 20) {
		    do {	/* symbol is longer than 6 bits */
			symbol = decr->huffman20_table[((control >> 6) & 1) +
						  (symbol << 1)];
			if (!shift--) {
			    shift += 16;
			    control += *src++ << 24;
			    control += *src++ << 16;
			}
			control >>= 1;
		    } while (symbol >= 20);
		    temp = 6;
		} else {
		    temp = decr->huffman20_len[symbol];
		}

		control >>= temp;
		if ((shift -= temp) < 0) {
		    shift += 16;
		    control += *src++ << (8 + shift);
		    control += *src++ << shift;
		}

		switch (symbol) {
		case 17:
		case 18:
		    if (symbol == 17) {
			temp = 4;
			count = 3;
		    } else {	/* symbol == 18 */
			temp = 6 - fix;
			count = 19;
		    }

		    count += (control & table_three[temp]) + fix;
		    control >>= temp;

		    if ((shift -= temp) < 0) {
			shift += 16;
			control += *src++ << (8 + shift);
			control += *src++ << shift;
		    }

		    while ((pos < max_symbol) && (count--))
			decr->literal_len[pos++] = 0;

		    break;
		case 19:
		    count = (control & 1) + 3 + fix;
		    if (!shift--) {
			shift += 16;
			control += *src++ << 24;
			control += *src++ << 16;
		    }

		    control >>= 1;
		    if ((symbol = decr->huffman20_table[control & 63]) >= 20) {
			do {	/* symbol is longer than 6 bits */
			    symbol =
				decr->huffman20_table[((control >> 6) & 1) +
						      (symbol << 1)];
			    if (!shift--) {
				shift += 16;
				control += *src++ << 24;
				control += *src++ << 16;
			    }
			    control >>= 1;
			} while (symbol >= 20);
			temp = 6;
		    } else {
			temp = decr->huffman20_len[symbol];
		    }

		    control >>= temp;

		    if ((shift -= temp) < 0) {
			shift += 16;
			control += *src++ << (8 + shift);
			control += *src++ << shift;
		    }

                    /* Sanity check */
                    if (pos >= 768)
                        return -1;

                    x = decr->literal_len[pos] + 17 - symbol;

                    /* Sanity check */
                    if (x >= 34)
                        return -1;

		    symbol = table_four[x];

		    while (pos < max_symbol && count--)
			decr->literal_len[pos++] = symbol;

		    break;
		default:
		    symbol = table_four[decr->literal_len[pos] + 17 - symbol];
		    decr->literal_len[pos++] = symbol;
		    break;
		}
	    } while (pos < max_symbol);

	    fix--;
	    max_symbol += 512;
	} while (max_symbol == 768);

	if (!abort)
	    abort = make_decode_table(768, 12, decr->literal_len,
				  decr->literal_table);
    }

    decr->control = control;
    decr->shift = shift;
    decr->src = src;

    return abort;
}

/* ---------------------------------------------------------------------- */

/* Fill up the decrunch buffer. Needs lots of overrun for both decr->dest */
/* and src buffers. Most of the time is spent in this routine so it's  */
/* pretty damn optimized. */

static void decrunch(struct LZXDecrData *decr)
{
    register uint32 control;
    register int shift;
    uint32 temp;		/* could be a register */
    uint32 symbol, count;
    uint8 *string, *src, *dest;

    control = decr->control;
    shift = decr->shift;
    src = decr->src;
    dest = decr->dest;

    do {
	if ((symbol = decr->literal_table[control & 4095]) >= 768) {
	    control >>= 12;

	    if ((shift -= 12) < 0) {
		shift += 16;
		control += *src++ << (8 + shift);
		control += *src++ << shift;
	    }
	    do {		/* literal is longer than 12 bits */
		symbol = decr->literal_table[(control & 1) + (symbol << 1)];
		if (!shift--) {
		    shift += 16;
		    control += *src++ << 24;
		    control += *src++ << 16;
		}
		control >>= 1;
	    } while (symbol >= 768);
	} else {
	    temp = decr->literal_len[symbol];
	    control >>= temp;

	    if ((shift -= temp) < 0) {
		shift += 16;
		control += *src++ << (8 + shift);
		control += *src++ << shift;
	    }
	}

	if (symbol < 256) {
	    *dest++ = symbol;
	} else {
	    symbol -= 256;
	    count = table_two[temp = symbol & 31];
	    temp = table_one[temp];

	    if ((temp >= 3) && (decr->method == 3)) {
		temp -= 3;
		count += ((control & table_three[temp]) << 3);
		control >>= temp;
		if ((shift -= temp) < 0) {
		    shift += 16;
		    control += *src++ << (8 + shift);
		    control += *src++ << shift;
		}
		count += (temp = decr->offset_table[control & 127]);
		temp = decr->offset_len[temp];
	    } else {
		count += control & table_three[temp];
		if (!count)
		    count = decr->last_offset;
	    }

	    control >>= temp;

	    if ((shift -= temp) < 0) {
		shift += 16;
		control += *src++ << (8 + shift);
		control += *src++ << shift;
	    }

	    decr->last_offset = count;

	    count = table_two[temp = (symbol >> 5) & 15] + 3;
	    temp = table_one[temp];
	    count += (control & table_three[temp]);
	    control >>= temp;

	    shift -= temp;
	    if (shift < 0) {
		shift += 16;
		control += *src++ << (8 + shift);
		control += *src++ << shift;
	    }

	    string = (decr->buffer + decr->last_offset < dest) ?
			dest - decr->last_offset :
			dest + 65536 - decr->last_offset;

	    do {
		*dest++ = *string++;
	    } while (--count);
	}
    } while (dest < decr->dest_end && src < decr->src_end);

    decr->control = control;
    decr->shift = shift;
    decr->src = src;
    decr->dest = dest;
}

/* ---------------------------------------------------------------------- */

/* Trying to understand this function is hazardous. */

static int extract_normal(FILE * in_file, struct LZXDecrData *decr)
{
    struct filename_node *node;
    FILE *out_file = 0;
    uint8 *pos;
    uint8 *temp;
    uint32 count;
    int abort = 0;

    decr->control = 0;	/* initial control word */
    decr->shift = -16;
    decr->last_offset = 1;
    decr->unpack_size = 0;
    decr->decrunch_length = 0;

    memset(decr->offset_len, 0, 8);
    memset(decr->literal_len, 0, 768);

    decr->src = decr->read_buffer + 16384;
    decr->src_end = decr->src - 1024;
    pos = decr->dest_end = decr->dest = decr->buffer + 258 + 65536;

    for (node = decr->filename_list; (!abort) && node; node = node->next) {
	/*printf("Extracting \"%s\"...", node->filename);
	   fflush(stdout); */

	if (exclude_match(node->filename)) {
	    out_file = NULL;
	} else {
	    out_file = decr->outfile;
	}

	decr->sum = 0;		/* reset CRC */
	decr->unpack_size = node->length;

	while (decr->unpack_size > 0) {

	    if (pos == decr->dest) {	/* time to fill the buffer? */
/* check if we have enough data and read some if not */
		if (decr->src >= decr->src_end) {	/* have we exhausted the current read buffer? */
		    temp = decr->read_buffer;
		    if ((count = temp - decr->src + 16384)) {
			do {	/* copy the remaining overrun to the start of the buffer */
			    *temp++ = *decr->src++;
			} while (--count);
		    }
		    decr->src = decr->read_buffer;
		    count = decr->src - temp + 16384;

		    if (decr->pack_size < count)
			count = decr->pack_size;	/* make sure we don't read too much */

		    if (fread(temp, 1, count, in_file) != count) {
			/* printf("\n");
			if (ferror(in_file))
			    perror("FRead(Data)");
			else
			    fprintf(stderr, "EOF: Data\n"); */
			abort = 1;
			break;	/* fatal error */
		    }
		    decr->pack_size -= count;

		    temp += count;
		    if (decr->src >= temp)
			break;	/* argh! no more data! */
		}

		/* if(src >= decr->src_end) */
		/* check if we need to read the tables */
		if (decr->decrunch_length <= 0) {
		    if (read_literal_table(decr))
			break;	/* argh! can't make huffman tables! */
		}

                /* unpack some data */
		if (decr->dest >= decr->buffer + 258 + 65536) {
		    if ((count =
			 decr->dest - decr->buffer - 65536)) {
			temp = (decr->dest =
				decr->buffer) + 65536;
			do {	/* copy the overrun to the start of the buffer */
			    *decr->dest++ = *temp++;
			} while (--count);
		    }
		    pos = decr->dest;
		}
		decr->dest_end = decr->dest + decr->decrunch_length;
		if (decr->dest_end > decr->buffer + 258 + 65536)
		    decr->dest_end = decr->buffer + 258 + 65536;
		temp = decr->dest;

		decrunch(decr);

		decr->decrunch_length -= (decr->dest - temp);
	    }

            /* calculate amount of data we can use before we need to
             * fill the buffer again
             */
	    count = decr->dest - pos;
	    if (count > decr->unpack_size)
		count = decr->unpack_size;	/* take only what we need */

	    decr->sum = crc32_A1(pos, count, decr->sum);

	    if (out_file) {	/* Write the data to the file */
		abort = 1;
		if (fwrite(pos, 1, count, out_file) != count) {
#if 0
		    perror("FWrite");	/* argh! write error */
		    fclose(out_file);
		    out_file = 0;
#endif
		}
	    }
	    decr->unpack_size -= count;
	    pos += count;
	}

#if 0
	if (out_file) {
	    fclose(out_file);
	    if (!abort)
		printf(" crc %s\n", (node->crc == sum) ? "good" : "bad");
	}
#endif
    }				/* for */

    return (abort);
}

/* ---------------------------------------------------------------------- */

static int extract_archive(FILE * in_file, struct LZXDecrData *decr)
{
    uint32 temp;
    struct filename_node **filename_next;
    struct filename_node *node;
    struct filename_node *temp_node;
    int actual;
    int abort;
    int result = 1;		/* assume an error */

    decr->filename_list = 0;	/* clear the list */
    filename_next = &decr->filename_list;

    do {
	abort = 1;		/* assume an error */
	actual = fread(decr->archive_header, 1, 31, in_file);
	if (ferror(in_file)) {
	    /* perror("FRead(Archive_Header)"); */
	    continue;
	}

	if (actual == 0) {	/* 0 is normal and means EOF */
	    result = 0;		/* normal termination */
	    continue;
	}

	if (actual != 31) {
	    /* fprintf(stderr, "EOF: Archive_Header\n"); */
	    continue;
	}

	decr->sum = 0;		/* reset CRC */
	decr->crc = readmem32l(decr->archive_header + 26);

	/* Must set the field to 0 before calculating the crc */
	memset(decr->archive_header + 26, 0, 4);
	decr->sum = crc32_A1(decr->archive_header, 31, decr->sum);
	temp = decr->archive_header[30];	/* filename length */
	actual = fread(decr->header_filename, 1, temp, in_file);

	if (ferror(in_file)) {
	    /* perror("FRead(Header_Filename)"); */
	    continue;
	}

	if (actual != temp) {
	    /* fprintf(stderr, "EOF: Header_Filename\n"); */
	    continue;
	}

	decr->header_filename[temp] = 0;
	decr->sum = crc32_A1(decr->header_filename, temp, decr->sum);
	temp = decr->archive_header[14];	/* comment length */
	actual = fread(decr->header_comment, 1, temp, in_file);

	if (ferror(in_file)) {
	    /* perror("FRead(Header_Comment)"); */
	    continue;
	}

	if (actual != temp) {
	    /* fprintf(stderr, "EOF: Header_Comment\n"); */
	    continue;
	}

	decr->header_comment[temp] = 0;
	decr->sum = crc32_A1(decr->header_comment, temp, decr->sum);

	if (decr->sum != decr->crc) {
	    /* fprintf(stderr, "CRC: Archive_Header\n"); */
	    continue;
	}

	decr->unpack_size = readmem32l(decr->archive_header + 2);
	decr->pack_size = readmem32l(decr->archive_header + 6);
	decr->pack_mode = decr->archive_header[11];
	decr->crc = readmem32l(decr->archive_header + 22);

	/* allocate a filename node */
	node = malloc(sizeof(struct filename_node));
	if (node == NULL) {
	    /* fprintf(stderr, "MAlloc(Filename_node)\n"); */
	    continue;
	}

	*filename_next = node;	/* add this node to the list */
	filename_next = &(node->next);
	node->next = 0;
	node->length = decr->unpack_size;
	node->crc = decr->crc;
	for (temp = 0; (node->filename[temp] = decr->header_filename[temp]);
	     temp++) ;

#if 0
	if (decr->pack_size == 0) {
	    abort = 0;		/* continue */
	    continue;
	}
#endif

	switch (decr->pack_mode) {
#if 0
	case 0:			/* store */
	    /*abort =*/ extract_store(in_file, decr);
	    abort = 1;		/* for xmp */
	    break;
#endif
	case 2:			/* normal */
	    /*abort =*/ extract_normal(in_file, decr);
	    abort = 1;		/* for xmp */
	    break;
	default:		/* unknown */
	    break;
	}

	if (abort)
	    break;		/* a read error occured */

	temp_node = decr->filename_list;	/* free the list now */
	while ((node = temp_node)) {
	    temp_node = node->next;
	    free(node);
	}
	decr->filename_list = 0;	/* clear the list */
	filename_next = &decr->filename_list;

	if (fseek(in_file, decr->pack_size, SEEK_CUR)) {
	    /* perror("FSeek(Data)"); */
	    break;
	}

    } while (!abort);

    /* free the filename list in case an error occured */
    temp_node = decr->filename_list;
    while ((node = temp_node)) {
	temp_node = node->next;
	free(node);
    }

    return result;
}

static int test_lzx(unsigned char *b)
{
	return memcmp(b, "LZX", 3) == 0;
}

static int decrunch_lzx(FILE *f, FILE *fo)
{
	struct LZXDecrData *decr;

	if (fo == NULL)
		return -1;

	decr = calloc(1, sizeof(struct LZXDecrData));
	if (decr == NULL)
		return -1;

	fseek(f, 10, SEEK_CUR);	/* skip header */

	crc32_init_A();
	decr->outfile = fo;
	extract_archive(f, decr);

	free(decr);

	return 0;
}

struct depacker lzx_depacker = {
	test_lzx,
	decrunch_lzx
};

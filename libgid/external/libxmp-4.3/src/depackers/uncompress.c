/* public domain decompress code */

#include "stdio2.h"
#include <string.h>
#include "depacker.h"

#define	MAGIC_1		31	/* First byte of compressed file */
#define	MAGIC_2		157	/* Second byte of compressed file */
#define BIT_MASK	0x1f	/* Mask for 'number of compresssion bits */
				/* Masks 0x20 and 0x40 are free. */
#define BITS		16
#define HSIZE		69001	/* 95% occupancy */
#define FIRST		257	/* first free entry */
#define CLEAR		256	/* table clear output code */
#define INIT_BITS	9	/* initial number of bits/code */
#define BLOCK_MODE	0x80	/* Block compresssion if table is full and */
#define MAXCODE(n)     	(1L << (n))
				/* compression rate is dropping flush tables */
#define IBUFSIZ		BUFSIZ	/* Default input buffer size */
#define OBUFSIZ		BUFSIZ	/* Default output buffer size */

#define	input(b,o,c,n,m) { \
	char_type *p = &(b)[(o)>>3]; \
	(c) = ((((long)(p[0]))|((long)(p[1])<<8)| \
	      ((long)(p[2])<<16))>>((o)&0x7))&(m); \
	      (o) += (n); \
} while (0)

typedef unsigned char char_type;
typedef long int code_int;
typedef long int count_int;
typedef long int cmp_code_int;


#define	htabof(i)		htab[i]
#define	codetabof(i)		codetab[i]
#define	tab_prefixof(i)		codetabof(i)
#define	tab_suffixof(i)		((char_type *)(htab))[i]
#define	de_stack		((char_type *)&(htab[HSIZE-1]))
#define	clear_htab()		memset(htab, -1, sizeof(htab))
#define	clear_tab_prefixof()	memset(codetab, 0, 256);

static int test_compress(unsigned char *b)
{
	return b[0] == 31 && b[1] == 157;
}

/*
 * Decompress stdin to stdout.  This routine adapts to the codes in the
 * file building the "string" table on-the-fly; requiring no table to
 * be stored in the compressed file.  The tables used herein are shared
 * with those of the compress() routine.  See the definitions above.
 */

static int decrunch_compress(FILE * in, FILE * out)
{
	char_type *stackp;
	code_int code;
	int finchar;
	code_int oldcode;
	code_int incode;
	int inbits;
	int posbits;
	int outpos;
	int insize;
	int bitmask;
	code_int free_ent;
	code_int maxcode;
	code_int maxmaxcode;
	int n_bits;
	int rsize;
	int maxbits;
	int block_mode;
	int i;

	long bytes_in;			/* Total number of byte from input */
	/*long bytes_out;*/		/* Total number of byte to output */
	char_type inbuf[IBUFSIZ + 64];	/* Input buffer */
	char_type outbuf[OBUFSIZ + 2048];/* Output buffer */
	count_int htab[HSIZE];
	unsigned short codetab[HSIZE];

	insize = 0;
	rsize = fread(inbuf, 1, IBUFSIZ, in);
	insize += rsize;

	if (insize < 3 || inbuf[0] != MAGIC_1 || inbuf[1] != MAGIC_2) {
		return -1;
	}

	maxbits = inbuf[2] & BIT_MASK;
	block_mode = inbuf[2] & BLOCK_MODE;
	maxmaxcode = MAXCODE(maxbits);

	if (maxbits > BITS) {
		/*fprintf(stderr,
		   "%s: compressed with %d bits, can only handle %d bits\n",
		   (*ifname != '\0' ? ifname : "stdin"), maxbits, BITS);
		   exit_code = 4; */
		return -1;
	}

	bytes_in = insize;
	maxcode = MAXCODE(n_bits = INIT_BITS) - 1;
	bitmask = (1 << n_bits) - 1;
	oldcode = -1;
	finchar = 0;
	outpos = 0;
	posbits = 3 << 3;

	free_ent = ((block_mode) ? FIRST : 256);

	clear_tab_prefixof();	/* As above, initialize the first
				   256 entries in the table. */

	for (code = 255; code >= 0; --code)
		tab_suffixof(code) = (char_type) code;

	do {
	      resetbuf:;
		{
			int i;
			int e;
			int o;

			o = posbits >> 3;
			e = o <= insize ? insize - o : 0;

			for (i = 0; i < e; ++i)
				inbuf[i] = inbuf[i + o];

			insize = e;
			posbits = 0;
		}

		if (insize < sizeof(inbuf) - IBUFSIZ) {
			if ((rsize = fread(inbuf + insize, 1, IBUFSIZ, in)) < 0)
				return -1;

			insize += rsize;
		}

		inbits = ((rsize > 0) ? (insize - insize % n_bits) << 3 :
			  (insize << 3) - (n_bits - 1));

		while (inbits > posbits) {
			if (free_ent > maxcode) {
				posbits = ((posbits - 1) + ((n_bits << 3) -
							    (posbits - 1 +
							     (n_bits << 3)) %
							    (n_bits << 3)));

				++n_bits;
				if (n_bits == maxbits)
					maxcode = maxmaxcode;
				else
					maxcode = MAXCODE(n_bits) - 1;

				bitmask = (1 << n_bits) - 1;
				goto resetbuf;
			}

			input(inbuf, posbits, code, n_bits, bitmask);

			if (oldcode == -1) {
				if (code >= 256) {
					fprintf(stderr, "oldcode:-1 code:%i\n",
						(int)(code));
					fprintf(stderr, "uncompress: corrupt input\n");
					/* abort_compress(); */
					return -1;
				}
				outbuf[outpos++] = (char_type)(finchar = (int)(oldcode = code));
				continue;
			}

			if (code == CLEAR && block_mode) {
				clear_tab_prefixof();
				free_ent = FIRST - 1;
				posbits = ((posbits - 1) + ((n_bits << 3) -
					    (posbits - 1 + (n_bits << 3)) %
							   (n_bits << 3)));
				maxcode = MAXCODE(n_bits = INIT_BITS) - 1;
				bitmask = (1 << n_bits) - 1;
				goto resetbuf;
			}

			incode = code;
			stackp = de_stack;

			if (code >= free_ent) {	/* Special case for KwKwK string. */
				if (code > free_ent) {
					/*char_type *p;

					posbits -= n_bits;
					p = &inbuf[posbits >> 3];

					fprintf(stderr,
						"insize:%d posbits:%d inbuf:%02X %02X %02X %02X %02X (%d)\n",
						insize, posbits, p[-1], p[0],
						p[1], p[2], p[3],
						(posbits & 07));*/
					fprintf(stderr,
						"uncompress: corrupt input\n");
					/* abort_compress(); */
					return -1;
				}

				*--stackp = (char_type) finchar;
				code = oldcode;
			}

			while ((cmp_code_int) code >= (cmp_code_int) 256) {	/* Generate output characters in reverse order */
				*--stackp = tab_suffixof(code);
				code = tab_prefixof(code);
			}

			*--stackp = (char_type) (finchar = tab_suffixof(code));

			/* And put them out in forward order */

			if (outpos + (i = (de_stack - stackp)) >= OBUFSIZ) {
				do {
					if (i > OBUFSIZ - outpos)
						i = OBUFSIZ - outpos;

					if (i > 0) {
						memcpy(outbuf + outpos, stackp, i);
						outpos += i;
					}

					if (outpos >= OBUFSIZ) {
						if (fwrite(outbuf, 1, outpos, out) != outpos) {
							return -1;
							/*write_error(); */
						}

						outpos = 0;
					}
					stackp += i;
				}
				while ((i = (de_stack - stackp)) > 0);
			} else {
				memcpy(outbuf + outpos, stackp, i);
				outpos += i;
			}

			if ((code = free_ent) < maxmaxcode) {	/* Generate the new entry. */
				tab_prefixof(code) = (unsigned short)oldcode;
				tab_suffixof(code) = (char_type) finchar;
				free_ent = code + 1;
			}

			oldcode = incode;	/* Remember previous code.      */
		}

		bytes_in += rsize;
	}
	while (rsize > 0);

	if (outpos > 0 && fwrite(outbuf, 1, outpos, out) != outpos)
		return -1;

	return 0;
}

struct depacker compress_depacker = {
	test_compress,
	decrunch_compress
};

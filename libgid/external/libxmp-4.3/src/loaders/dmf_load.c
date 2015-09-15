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

/*
 * Public domain DMF sample decompressor by Olivier Lapicque
 */

#include <assert.h>

#include "loader.h"
#include "iff.h"
#include "period.h"

#define MAGIC_DDMF	MAGIC4('D','D','M','F')


static int dmf_test(HIO_HANDLE *, char *, const int);
static int dmf_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader dmf_loader = {
	"X-Tracker",
	dmf_test,
	dmf_load
};

static int dmf_test(HIO_HANDLE * f, char *t, const int start)
{
	if (hio_read32b(f) != MAGIC_DDMF)
		return -1;

	hio_seek(f, 9, SEEK_CUR);
	read_title(f, t, 30);

	return 0;
}


struct local_data {
	int ver;
	uint8 packtype[256];
};


struct hnode {
	short int left, right;
	uint8 value;
};

struct htree {
	uint8 *ibuf, *ibufmax;
	uint32 bitbuf;
	int bitnum;
	int lastnode, nodecount;
	struct hnode nodes[256];
};


static uint8 read_bits(struct htree *tree, int nbits)
{
	uint8 x = 0, bitv = 1;
	while (nbits--) {
		if (tree->bitnum) {
			tree->bitnum--;
		} else {
			tree->bitbuf = (tree->ibuf < tree->ibufmax) ?
							*(tree->ibuf++) : 0;
			tree->bitnum = 7;
		}
		if (tree->bitbuf & 1) x |= bitv;
		bitv <<= 1;
		tree->bitbuf >>= 1;
	}
	return x;
}

/* tree: [8-bit value][12-bit index][12-bit index] = 32-bit */
static void new_node(struct htree *tree)
{
	uint8 isleft, isright;
	int actnode;

	actnode = tree->nodecount;

	if (actnode > 255)
		return;

	tree->nodes[actnode].value = read_bits(tree, 7);
	isleft = read_bits(tree, 1);
	isright = read_bits(tree, 1);
	actnode = tree->lastnode;

	if (actnode > 255)
		return;

	tree->nodecount++;
	tree->lastnode = tree->nodecount;

	if (isleft) {
		tree->nodes[actnode].left = tree->lastnode;
		new_node(tree);
	} else {
		tree->nodes[actnode].left = -1;
	}

	tree->lastnode = tree->nodecount;

	if (isright) {
		tree->nodes[actnode].right = tree->lastnode;
		new_node(tree);
	} else {
		tree->nodes[actnode].right = -1;
	}
}

static int unpack(uint8 *psample, uint8 *ibuf, uint8 *ibufmax, uint32 maxlen)
{
	struct htree tree;
	int i, actnode;
	uint8 value, sign, delta = 0;
	
	memset(&tree, 0, sizeof(tree));
	tree.ibuf = ibuf;
	tree.ibufmax = ibufmax;
	new_node(&tree);
	value = 0;

	for (i = 0; i < maxlen; i++) {
		actnode = 0;
		sign = read_bits(&tree, 1);

		do {
			if (read_bits(&tree, 1))
				actnode = tree.nodes[actnode].right;
			else
				actnode = tree.nodes[actnode].left;
			if (actnode > 255) break;
			delta = tree.nodes[actnode].value;
			if ((tree.ibuf >= tree.ibufmax) && (!tree.bitnum)) break;
		} while ((tree.nodes[actnode].left >= 0) &&
					(tree.nodes[actnode].right >= 0));

		if (sign)
			delta ^= 0xff;
		value += delta;
		psample[i] = i ? value : 0;
	}

	return tree.ibuf - ibuf;
}


/*
 * IFF chunk handlers
 */

static int get_sequ(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	int i;

	hio_read16l(f);	/* sequencer loop start */
	hio_read16l(f);	/* sequencer loop end */

	mod->len = (size - 4) / 2;
	if (mod->len > 255)
		mod->len = 255;

	for (i = 0; i < mod->len; i++)
		mod->xxo[i] = hio_read16l(f);

	return 0;
}

static int get_patt(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	int i, j, r, chn;
	int patsize;
	int info, counter, data;
	int track_counter[32];
	struct xmp_event *event;

	mod->pat = hio_read16l(f);
	mod->chn = hio_read8(f);
	mod->trk = mod->chn * mod->pat;

	if (pattern_init(mod) < 0)
		return -1;

	D_(D_INFO "Stored patterns: %d", mod->pat);

	for (i = 0; i < mod->pat; i++) {
		chn = hio_read8(f);
		hio_read8(f);		/* beat */

		if (pattern_tracks_alloc(mod, i, hio_read16l(f)) < 0)
			return -1;

		patsize = hio_read32l(f);

		for (j = 0; j < chn; j++)
			track_counter[j] = 0;

		for (counter = r = 0; r < mod->xxp[i]->rows; r++) {
			if (counter == 0) {
				/* global track */
				info = hio_read8(f);
				counter = info & 0x80 ? hio_read8(f) : 0;
				data = info & 0x3f ? hio_read8(f) : 0;
			} else {
				counter--;
			}

			for (j = 0; j < chn; j++) {
				int b, fxt, fxp;

				event = &EVENT(i, j, r);

				if (track_counter[j] == 0) {
					b = hio_read8(f);
		
					if (b & 0x80)
						track_counter[j] = hio_read8(f);
					if (b & 0x40)
						event->ins = hio_read8(f);
					if (b & 0x20)
						event->note = 24 + hio_read8(f);
					if (b & 0x10)
						event->vol = hio_read8(f);
					if (b & 0x08) {	/* instrument effect */
						fxt = hio_read8(f);
						fxp = hio_read8(f);
					}
					if (b & 0x04) {	/* note effect */
						fxt = hio_read8(f);
						fxp = hio_read8(f);
					}
					if (b & 0x02) {	/* volume effect */
						fxt = hio_read8(f);
						fxp = hio_read8(f);
						switch (fxt) {
						case 0x02:
							event->fxt = FX_VOLSLIDE_DN;
							event->fxp = fxp;
							break;
						}
					}
				} else {
					track_counter[j]--;
				}
			}
		}
	}

	return 0;
}

static int get_smpi(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	struct local_data *data = (struct local_data *)parm;
	int i, namelen, c3spd, flag;
	uint8 name[30];

	mod->ins = mod->smp = hio_read8(f);

	if (instrument_init(mod) < 0)
		return -1;

	D_(D_INFO "Instruments: %d", mod->ins);

	for (i = 0; i < mod->ins; i++) {
		int x;

		if (subinstrument_alloc(mod, i, 1) < 0)
			return -1;
		
		namelen = hio_read8(f);
		x = namelen - hio_read(name, 1, namelen > 30 ? 30 : namelen, f);
		instrument_name(mod, i, name, namelen);
		name[namelen] = 0;
		while (x--)
			hio_read8(f);

		mod->xxs[i].len = hio_read32l(f);
		mod->xxs[i].lps = hio_read32l(f);
		mod->xxs[i].lpe = hio_read32l(f);
		mod->xxi[i].nsm = !!mod->xxs[i].len;
		c3spd = hio_read16l(f);
		c2spd_to_note(c3spd, &mod->xxi[i].sub[0].xpo, &mod->xxi[i].sub[0].fin);
		mod->xxi[i].sub[0].vol = hio_read8(f);
		mod->xxi[i].sub[0].pan = 0x80;
		mod->xxi[i].sub[0].sid = i;
		flag = hio_read8(f);
		mod->xxs[i].flg = flag & 0x01 ? XMP_SAMPLE_LOOP : 0;
		if (data->ver >= 8)
			hio_seek(f, 8, SEEK_CUR);	/* library name */
		hio_read16l(f);	/* reserved -- specs say 1 byte only*/
		hio_read32l(f);	/* sampledata crc32 */

		data->packtype[i] = (flag & 0x0c) >> 2;
		D_(D_INFO "[%2X] %-30.30s %05x %05x %05x %c P%c %5d V%02x",
				i, name, mod->xxs[i].len, mod->xxs[i].lps & 0xfffff,
				mod->xxs[i].lpe & 0xfffff,
				mod->xxs[i].flg & XMP_SAMPLE_LOOP ? 'L' : ' ',
				'0' + data->packtype[i],
				c3spd, mod->xxi[i].sub[0].vol);
	}

	return 0;
}

struct dynamic_buffer
{
	uint32 size;
	uint8* data;
};

static int dynamic_buffer_alloc(struct dynamic_buffer* buf, uint32 size)
{
	uint8* data;
	if (buf->size >= size)
	  return 0;
	if (!buf->data)
	  data = malloc(size);
	else
	  data = realloc(buf->data, size);
	if (data) {
	  buf->data = data;
	  buf->size = size;
	  return 0;
	} else {
	  return -1;
	}
}

static void dynamic_buffer_free(struct dynamic_buffer* buf)
{
	free(buf->data);
}

static int get_smpd(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	struct local_data *data = (struct local_data *)parm;
	int i;
	struct dynamic_buffer sbuf = {0}, ibuf = {0};

	D_(D_INFO "Stored samples: %d", mod->smp);

	for (i = 0; i < mod->smp; i++) {
		uint32 samplesize = mod->xxs[i].len;
		uint32 datasize = hio_read32l(f);
		if (datasize == 0)
			continue;

		switch (data->packtype[i]) {
		case 0:
      if (load_sample(m, f, 0, &mod->xxs[i], NULL) < 0)
				goto error;
			break;
		case 1:
      if (dynamic_buffer_alloc(&ibuf, datasize) < 0)
				goto error;
      if (hio_read(ibuf.data, 1, datasize, f) != datasize)
				goto error;
      if (dynamic_buffer_alloc(&sbuf, samplesize) < 0)
				goto error;
			unpack(sbuf.data, ibuf.data, ibuf.data + datasize, samplesize);
			if (load_sample(m, NULL, SAMPLE_FLAG_NOLOAD,
        &mod->xxs[i], (char *)sbuf.data) < 0)
        goto error;
			break;
		default:
			hio_seek(f, datasize, SEEK_CUR);
		}
	}
	dynamic_buffer_free(&ibuf);
	dynamic_buffer_free(&sbuf);
	return 0;
error:
	dynamic_buffer_free(&ibuf);
	dynamic_buffer_free(&sbuf);
	return -1;
}

static int dmf_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
	struct xmp_module *mod = &m->mod;
	iff_handle handle;
	uint8 date[3];
	char tracker_name[10];
	struct local_data data;
	int ret;

	LOAD_INIT();

	hio_read32b(f);		/* DDMF */

	data.ver = hio_read8(f);
	hio_read(tracker_name, 8, 1, f);
	tracker_name[8] = 0;
	snprintf(mod->type, XMP_NAME_SIZE, "%s DMF v%d",
				tracker_name, data.ver);
	tracker_name[8] = 0;
	hio_read(mod->name, 30, 1, f);
	hio_seek(f, 20, SEEK_CUR);
	hio_read(date, 3, 1, f);
	
	MODULE_INFO();
	D_(D_INFO "Creation date: %02d/%02d/%04d", date[0],
						date[1], 1900 + date[2]);
	
	handle = iff_new();
	if (handle == NULL)
		return -1;

	/* IFF chunk IDs */
	ret = iff_register(handle, "SEQU", get_sequ);
	ret |= iff_register(handle, "PATT", get_patt);
	ret |= iff_register(handle, "SMPI", get_smpi);
	ret |= iff_register(handle, "SMPD", get_smpd);

	if (ret != 0)
		return -1;

	iff_set_quirk(handle, IFF_LITTLE_ENDIAN);

	/* Load IFF chunks */
	if (iff_load(handle, m, f, &data) < 0) {
		iff_release(handle);
		return -1;
	}

	m->volbase = 0xff;

	iff_release(handle);

	return 0;
}

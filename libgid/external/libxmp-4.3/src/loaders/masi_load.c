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
 * Originally based on the PSM loader from Modplug by Olivier Lapicque and
 * fixed comparing the One Must Fall! PSMs with Kenny Chou's MTM files.
 */

/*
 * From EPICTEST Readme.1st:
 *
 * The Music And Sound Interface, MASI, is the basis behind all new Epic
 * games. MASI uses its own proprietary file format, PSM, for storing
 * its music.
 */

/*
 * kode54's comment on Sinaria PSMs in the foo_dumb hydrogenaudio forum:
 *
 * "The Sinaria variant uses eight character pattern and instrument IDs,
 * the sample headers are laid out slightly different, and the patterns
 * use a different format for the note values, and also different effect
 * scales for certain commands.
 *
 * [Epic] PSM uses high nibble for octave and low nibble for note, for
 * a valid range up to 0x7F, for a range of D-1 through D#9 compared to
 * IT. (...) Sinaria PSM uses plain note values, from 1 - 83, for a
 * range of C-3 through B-9.
 *
 * [Epic] PSM also uses an effect scale for portamento, volume slides,
 * and vibrato that is about four times as sensitive as the IT equivalents.
 * Sinaria does not. This seems to coincide with the MOD/S3M to PSM
 * converter that Joshua Jensen released in the EPICTEST.ZIP file which
 * can still be found on a few FTP sites. It converted effects literally,
 * even though the bundled players behaved as the libraries used with
 * Epic's games did and made the effects sound too strong."
 */

/*
 * Claudio's note: Sinaria seems to have a finetune byte just before
 * volume, slightly different sample size (subtract 2 bytes?) and some
 * kind of (stereo?) interleaved sample, with 16-byte frames (see Sinaria
 * songs 5 and 8). Sinaria song 10 sounds ugly, possibly caused by wrong
 * pitchbendings (see note above).
 */

/* FIXME: TODO: sinaria effects */

#include "loader.h"
#include "iff.h"
#include "period.h"

#define MAGIC_PSM_	MAGIC4('P','S','M',' ')
#define MAGIC_FILE	MAGIC4('F','I','L','E')
#define MAGIC_TITL	MAGIC4('T','I','T','L')
#define MAGIC_OPLH	MAGIC4('O','P','L','H')


static int masi_test (HIO_HANDLE *, char *, const int);
static int masi_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader masi_loader = {
	"Epic MegaGames MASI",
	masi_test,
	masi_load
};

static int masi_test(HIO_HANDLE *f, char *t, const int start)
{
	int val;

	if (hio_read32b(f) != MAGIC_PSM_)
		return -1;

	hio_read8(f);
	hio_read8(f);
	hio_read8(f);
	if (hio_read8(f) != 0)
		return -1;

	if (hio_read32b(f) != MAGIC_FILE) 
		return -1;

	hio_read32b(f);
	val = hio_read32l(f);
	hio_seek(f, val, SEEK_CUR);

	if (hio_read32b(f) == MAGIC_TITL) {
		val = hio_read32l(f);
		read_title(f, t, val);
	} else {
		read_title(f, t, 0);
	}

	return 0;
}

struct local_data {
	int sinaria;
	int cur_pat;
	int cur_ins;
	uint8 *pnam;
	uint8 *pord;
};

static int get_sdft(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	return 0;
}

static int get_titl(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	char buf[40];
	
	hio_read(buf, 1, 40, f);
	strncpy(mod->name, buf, size > 32 ? 32 : size);

	return 0;
}

static int get_dsmp_cnt(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;

	mod->ins++;
	mod->smp = mod->ins;

	return 0;
}

static int get_pbod_cnt(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	struct local_data *data = (struct local_data *)parm;
	char buf[20];

	mod->pat++;
	hio_read(buf, 1, 20, f);
	if (buf[9] != 0 && buf[13] == 0)
		data->sinaria = 1;

	return 0;
}


static int get_dsmp(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	struct xmp_instrument *xxi;
	struct xmp_subinstrument *sub;
	struct xmp_sample *xxs;
	struct local_data *data = (struct local_data *)parm;
	int i, srate;
	int finetune;

	hio_read8(f);					/* flags */
	hio_seek(f, 8, SEEK_CUR);			/* songname */
	hio_seek(f, data->sinaria ? 8 : 4, SEEK_CUR);	/* smpid */

	i = data->cur_ins;
	if (subinstrument_alloc(mod, i, 1) < 0)
		return -1;

	xxi = &mod->xxi[i];
	sub = &xxi->sub[0];
	xxs = &mod->xxs[i];

	hio_read(&xxi->name, 1, 31, f);
	adjust_string((char *)xxi->name);
	hio_seek(f, 8, SEEK_CUR);
	hio_read8(f);		/* insno */
	hio_read8(f);
	xxs->len = hio_read32l(f);
	xxs->lps = hio_read32l(f);
	xxs->lpe = hio_read32l(f);
	xxs->flg = xxs->lpe > 2 ? XMP_SAMPLE_LOOP : 0;
	hio_read16l(f);

	if ((int32)xxs->lpe < 0)
		xxs->lpe = 0;

	if (xxs->len > 0)
		xxi->nsm = 1;

	finetune = 0;
	if (data->sinaria) {
		if (xxs->len > 2)
			xxs->len -= 2;
		if (xxs->lpe > 2)
			xxs->lpe -= 2;

		finetune = (int8)(hio_read8s(f) << 4);
	}

	sub->vol = hio_read8(f) / 2 + 1;
	hio_read32l(f);
	sub->pan = 0x80;
	sub->sid = i;
	srate = hio_read32l(f);

	D_(D_INFO "[%2X] %-32.32s %05x %05x %05x %c V%02x %+04d %5d", i,
		xxi->name, xxs->len, xxs->lps, xxs->lpe,
		xxs->flg & XMP_SAMPLE_LOOP ?  'L' : ' ',
		sub->vol, finetune, srate);

	srate = 8363 * srate / 8448;
	c2spd_to_note(srate, &sub->xpo, &sub->fin);
	sub->fin += finetune;

	hio_seek(f, 16, SEEK_CUR);
	if (load_sample(m, f, SAMPLE_FLAG_8BDIFF, xxs, NULL) < 0)
		return -1;

	data->cur_ins++;

	return 0;
}


static int get_pbod(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	struct local_data *data = (struct local_data *)parm;
	int i, r;
	struct xmp_event *event, dummy;
	uint8 flag, chan;
	/* uint32 len; */
	int rows, rowlen;

	i = data->cur_pat;

	/*len =*/ hio_read32l(f);
	hio_read(data->pnam + i * 8, 1, data->sinaria ? 8 : 4, f);

	rows = hio_read16l(f);

	if (pattern_tracks_alloc(mod, i, rows) < 0)
		return -1;

	r = 0;

	do {
		rowlen = hio_read16l(f) - 2;
		while (rowlen > 0) {
			flag = hio_read8(f);
	
			if (rowlen == 1)
				break;
	
			chan = hio_read8(f);
			rowlen -= 2;
	
			event = chan < mod->chn ? &EVENT(i, chan, r) : &dummy;
	
			if (flag & 0x80) {
				uint8 note = hio_read8(f);
				rowlen--;
				if (data->sinaria)
					note += 36;
				else
					note = (note >> 4) * 12 + (note & 0x0f) + 1 + 12;
				event->note = note;
			}

			if (flag & 0x40) {
				event->ins = hio_read8(f) + 1;
				rowlen--;
			}
	
			if (flag & 0x20) {
				event->vol = hio_read8(f) / 2 + 1;
				rowlen--;
			}
	
			if (flag & 0x10) {
				uint8 fxt = hio_read8(f);
				uint8 fxp = hio_read8(f);
				rowlen -= 2;
	
				/* compressed events */
				if (fxt >= 0x40) {
					switch (fxp >> 4) {
					case 0x0: {
						uint8 note;
						note = (fxt>>4)*12 +
							(fxt & 0x0f) + 1;
						event->note = note;
						fxt = FX_TONEPORTA;
						fxp = (fxp + 1) * 2;
						break; }
					default:
D_(D_CRIT "p%d r%d c%d: compressed event %02x %02x\n", i, r, chan, fxt, fxp);
						return -1;
					}
				} else
				switch (fxt) {
				case 0x01:		/* fine volslide up */
					fxt = FX_EXTENDED;
					fxp = (EX_F_VSLIDE_UP << 4) |
						((fxp / 2) & 0x0f);
					break;
				case 0x02:		/* volslide up */
					fxt = FX_VOLSLIDE;
					fxp = (fxp / 2) << 4;
					break;
				case 0x03:		/* fine volslide down */
					fxt = FX_EXTENDED;
					fxp = (EX_F_VSLIDE_DN << 4) |
						((fxp / 2) & 0x0f);
					break;
				case 0x04: 		/* volslide down */
					fxt = FX_VOLSLIDE;
					fxp /= 2;
					break;
			    	case 0x0C:		/* portamento up */
					fxt = FX_PORTA_UP;
					fxp = (fxp - 1) / 2;
					break;
				case 0x0E:		/* portamento down */
					fxt = FX_PORTA_DN;
					fxp = (fxp - 1) / 2;
					break;
				case 0x0f:		/* tone portamento */
					fxt = FX_TONEPORTA;
					fxp /= 4;
					break;
				case 0x15:		/* vibrato */
					fxt = data->sinaria ?
						FX_VIBRATO : FX_FINE_VIBRATO;
					/* fxp remains the same */
					break;
				case 0x2a:		/* retrig note */
					fxt = FX_EXTENDED;
					fxp = (EX_RETRIG << 4) | (fxp & 0x0f); 
					break;
				case 0x29:		/* unknown */
					hio_read16l(f);
					rowlen -= 2;
					break;
				case 0x33:		/* position Jump */
					fxt = FX_JUMP;
					break;
			    	case 0x34:		/* pattern break */
					fxt = FX_BREAK;
					break;
				case 0x3D:		/* speed */
					fxt = FX_SPEED;
					break;
				case 0x3E:		/* tempo */
					fxt = FX_SPEED;
					break;
				default:
D_(D_CRIT "p%d r%d c%d: unknown effect %02x %02x\n", i, r, chan, fxt, fxp);
					fxt = fxp = 0;
				}
	
				event->fxt = fxt;
				event->fxp = fxp;
			}
		}
		r++;
	} while (r < rows);

	data->cur_pat++;

	return 0;
}

static int get_song(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;

	hio_seek(f, 10, SEEK_CUR);
	mod->chn = hio_read8(f);

	return 0;
}

static int get_song_2(struct module_data *m, int size, HIO_HANDLE *f, void *parm)
{
	struct xmp_module *mod = &m->mod;
	struct local_data *data = (struct local_data *)parm;
	uint32 magic;
	char c, buf[20];
	int i;

	hio_read(buf, 1, 9, f);
	hio_read16l(f);

	D_(D_INFO "Subsong title: %-9.9s", buf);

	magic = hio_read32b(f);
	while (magic != MAGIC_OPLH) {
		int skip;
		skip = hio_read32l(f);;
		hio_seek(f, skip, SEEK_CUR);
		magic = hio_read32b(f);
	}

	hio_read32l(f);	/* chunk size */

	hio_seek(f, 9, SEEK_CUR);		/* unknown data */
	
	c = hio_read8(f);
	for (i = 0; c != 0x01; c = hio_read8(f)) {
		switch (c) {
		case 0x07:
			mod->spd = hio_read8(f);
			hio_read8(f);		/* 08 */
			mod->bpm = hio_read8(f);
			break;
		case 0x0d:
			hio_read8(f);		/* channel number? */
			mod->xxc[i].pan = hio_read8(f);
			hio_read8(f);		/* flags? */
			i++;
			break;
		case 0x0e:
			hio_read8(f);		/* channel number? */
			hio_read8(f);		/* ? */
			break;
		default:
			/*printf("channel %d: %02x %02x\n", i, c, hio_read8(f));*/
			return -1;
		}
	}

	for (; c == 0x01; c = hio_read8(f)) {
		hio_read(data->pord + mod->len * 8, 1, data->sinaria ? 8 : 4, f);
		mod->len++;
	}

	return 0;
}

static int masi_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
	struct xmp_module *mod = &m->mod;
	iff_handle handle;
	int ret, offset;
	int i, j;
	struct local_data data;

	LOAD_INIT();

	hio_read32b(f);

	data.sinaria = 0;
	mod->name[0] = 0;

	hio_seek(f, 8, SEEK_CUR);		/* skip file size and FILE */
	mod->smp = mod->ins = 0;
	data.cur_pat = 0;
	data.cur_ins = 0;
	offset = hio_tell(f);

	handle = iff_new();
	if (handle == NULL)
		goto err;

	/* IFF chunk IDs */
	ret = iff_register(handle, "TITL", get_titl);
	ret |= iff_register(handle, "SDFT", get_sdft);
	ret |= iff_register(handle, "SONG", get_song);
	ret |= iff_register(handle, "DSMP", get_dsmp_cnt);
	ret |= iff_register(handle, "PBOD", get_pbod_cnt);

	if (ret != 0)
		goto err;

	iff_set_quirk(handle, IFF_LITTLE_ENDIAN);

	/* Load IFF chunks */
	if (iff_load(handle, m, f, &data) < 0) {
		iff_release(handle);
		goto err;
	}

	iff_release(handle);

	mod->trk = mod->pat * mod->chn;
	data.pnam = malloc(mod->pat * 8);	/* pattern names */
	if (data.pnam == NULL)
		goto err;

	data.pord = malloc(255 * 8);		/* pattern orders */
	if (data.pord == NULL)
		goto err2;

	set_type(m, data.sinaria ?
		"Sinaria PSM" : "Epic MegaGames MASI PSM");

	MODULE_INFO();

	if (instrument_init(mod) < 0)
		goto err3;

	if (pattern_init(mod) < 0)
		goto err3;

	D_(D_INFO "Stored patterns: %d", mod->pat);
	D_(D_INFO "Stored samples : %d", mod->smp);

	hio_seek(f, start + offset, SEEK_SET);

	mod->len = 0;

	handle = iff_new();
	if (handle == NULL)
		goto err3;

	/* IFF chunk IDs */
	ret = iff_register(handle, "SONG", get_song_2);
	ret |= iff_register(handle, "DSMP", get_dsmp);
	ret |= iff_register(handle, "PBOD", get_pbod);

	if (ret != 0)
		goto err3;

	iff_set_quirk(handle, IFF_LITTLE_ENDIAN);

	/* Load IFF chunks */
	if (iff_load(handle, m, f, &data) < 0) {
		iff_release(handle);
		goto err3;
	}

	iff_release(handle);

	for (i = 0; i < mod->len; i++) {
		for (j = 0; j < mod->pat; j++) {
			if (!memcmp(data.pord + i * 8, data.pnam + j * 8, data.sinaria ? 8 : 4)) {
				mod->xxo[i] = j;
				break;
			}
		}

		if (j == mod->pat)
			break;
	}

	free(data.pord);
	free(data.pnam);

	return 0;

    err3:
	free(data.pord);
    err2:
	free(data.pnam);
    err:
	return -1;
}

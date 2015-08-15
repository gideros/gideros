/* Extended Module Player
 * Copyright (C) 1996-2015 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

#define D_EBUG

#include "loader.h"

#define MAGIC_HVL	MAGIC4('H','V','L', 0)

static int hvl_test (HIO_HANDLE *, char *, const int);
static int hvl_load (struct module_data *, HIO_HANDLE *, const int);


const struct format_loader hvl_loader = {
	"HVL",
	"Hively Tracker",
	hvl_test,
	hvl_load
};

static int hvl_test(HIO_HANDLE *f, char *t, const int start)
{
	if (hio_read32b(f) != MAGIC_HVL)
		return -1;

	uint16 off = hio_read16b(f);
	if (hio_seek(f, off + 1, SEEK_SET))
		return -1;

	read_title(f, t, 32);

	return 0;
}


static void hvl_GenSawtooth(int8 * buf, uint32 len)
{
	uint32 i;
	int32 val, add;

	add = 256 / (len - 1);
	val = -128;

	for (i = 0; i < len; i++, val += add)
		*buf++ = (int8) val;
}

static void hvl_GenTriangle(int8 * buf, uint32 len)
{
	uint32 i;
	int32 d2, d5, d1, d4;
	int32 val;
	int8 *buf2;

	d2 = len;
	d5 = len >> 2;
	d1 = 128 / d5;
	d4 = -(d2 >> 1);
	val = 0;

	for (i = 0; i < d5; i++) {
		*buf++ = val;
		val += d1;
	}
	*buf++ = 0x7f;

	if (d5 != 1) {
		val = 128;
		for (i = 0; i < d5 - 1; i++) {
			val -= d1;
			*buf++ = val;
		}
	}

	buf2 = buf + d4;
	for (i = 0; i < d5 * 2; i++) {
		int8 c;

		c = *buf2++;
		if (c == 0x7f)
			c = 0x80;
		else
			c = -c;

		*buf++ = c;
	}
}

static void hvl_GenSquare(int8 * buf)
{
	uint32 i, j;

	for (i = 1; i <= 0x20; i++) {
		for (j = 0; j < (0x40 - i) * 2; j++)
			*buf++ = 0x80;
		for (j = 0; j < i * 2; j++)
			*buf++ = 0x7f;
	}
}

static void hvl_GenWhiteNoise(int8 * buf, uint32 len)
{
	uint32 ays;

	ays = 0x41595321;

	do {
		uint16 ax, bx;
		int8 s;

		s = ays;

		if (ays & 0x100) {
			s = 0x80;

			if ((int32) (ays & 0xffff) >= 0)
				s = 0x7f;
		}

		*buf++ = s;
		len--;

		ays = (ays >> 5) | (ays << 27);
		ays = (ays & 0xffffff00) | ((ays & 0xff) ^ 0x9a);
		bx = ays;
		ays = (ays << 2) | (ays >> 30);
		ax = ays;
		bx += ax;
		ax ^= bx;
		ays = (ays & 0xffff0000) | ax;
		ays = (ays >> 3) | (ays << 29);
	} while (len);
}

static void fix_effect (uint8 *fx, uint8 *param) {
	switch (*fx) {
	case 0:
		if (*param)
			*fx = FX_AHX_FILTER;
		break;
	case 3: /* init square */
		*fx = FX_AHX_SQUARE;
		break;
	case 4: /* set filter */
		*fx = FX_AHX_MODULATE;
		break;
	case 6: /* unused */
	case 8: /* unused */
		*fx = *param = 0;
		break;
	case 7:
		*fx = FX_MASTER_PAN;
		*param ^= 0x80;
//		printf ("pan %02x\n", *param);
		break;
	case 9:
		D_(D_INFO "square %02x", *param);
		break;
	case 12:
		if (*param >= 0x50 && *param <= 0x90) {
			*param -= 0x50;
			*fx = FX_GLOBALVOL;
		} else if (*param >= 0xa0 && *param <= 0xe0) {
			*param -= 0xa0;
			*fx = FX_TRK_VOL;
		}
		break;
	case 15:
		*fx = FX_S3M_SPEED;
		break;
	}
}

static int hvl_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
	struct player_data *p = &ctx->p;
	struct xmp_module *mod = &m->mod;
	int i, j, tmp, blank;

	LOAD_INIT();

	hio_read32b(f);

	uint16 title_offset = hio_read16b(f);
	tmp = hio_read16b(f);
	mod->len = tmp & 0xfff;
	blank = tmp & 0x8000;
		
	tmp = hio_read16b(f);
	mod->chn = (tmp >> 10) + 4;
	mod->rst = tmp & 1023;

	int pattlen = hio_read8(f);
	mod->trk = hio_read8(f) + 1;
	mod->ins = hio_read8(f);
	int subsongs = hio_read8(f);
	int gain = hio_read8(f);
	int stereo = hio_read8(f);

	D_(D_WARN "pattlen=%d npatts=%d nins=%d seqlen=%d stereo=%02x",
		pattlen, mod->trk, mod->ins, mod->len, stereo);

	set_type(m, "HVL (Hively Tracker)");
	MODULE_INFO();

	mod->pat = mod->len;
	mod->smp = 20;
	PATTERN_INIT();
	INSTRUMENT_INIT();

	hio_seek (f, subsongs*2, SEEK_CUR);

	uint8 *seqbuf = malloc(mod->len * mod->chn * 2);
	uint8 *seqptr = seqbuf;
	hio_read (seqbuf, 1, mod->len * mod->chn * 2, f);

	uint8 **transbuf = malloc (mod->len * mod->chn * sizeof(uint8 *));
	int transposed = 0;

	reportv(ctx, 0, "Stored patterns: %d ", mod->len);

	for (i = 0; i < mod->len; i++) {
		PATTERN_ALLOC(i);
		mod->xxp[i]->rows = pattlen;
		for (j = 0; j < mod->chn; j++) {
			if (seqptr[1]) {
//				printf ("%d: transpose %02x by %d\n", i, seqptr[0], seqptr[1]);
				mod->xxp[i]->info[j].index = mod->trk + transposed;
				transbuf[transposed] = seqptr;
				transposed++;
			} else {
				mod->xxp[i]->info[j].index = seqptr[0];
			}
			seqptr += 2;
//			printf ("%02x ", mod->xxp[i]->info[j].index);
		}
//		printf ("\n");
		mod->xxo[i] = i;
		reportv(ctx, 0, ".");
	}
	reportv(ctx, 0, "\n");
	

	/*
	 * tracks
	 */

	if (transposed) {
		mod->trk += transposed;
		mod->xxt = realloc(mod->xxt, mod->trk * sizeof (struct xmp_track *));
	}
	
	reportv(ctx, 0, "Stored tracks  : %d ", mod->trk);

	for (i = 0; i < mod->trk; i++) {
		mod->xxt[i] = calloc(sizeof(struct xmp_track) +
				   sizeof(struct xmp_event) * pattlen - 1, 1);
                mod->xxt[i]->rows = pattlen;

		if (!i && blank)
			continue;

		if (i >= mod->trk-transposed) {
			int n=i-(mod->trk - transposed);
			int o=transbuf[n][1];
			if (o>127)
				o-=256;
//			printf ("pattern %02x: source %02x offset %d\n", i, n, o);
			memcpy (mod->xxt[i], mod->xxt[transbuf[n][0]],
				sizeof(struct xmp_track) +
				sizeof(struct xmp_event) * pattlen - 1);
			for (j = 0; j < mod->xxt[i]->rows; j++) {
				struct xmp_event *event = &mod->xxt[i]->event[j];
				if (event->note)
					event->note+=o;
			}
			continue;
		}

		for (j = 0; j < mod->xxt[i]->rows; j++) {
			struct xmp_event *event = &mod->xxt[i]->event[j];
			int note = hio_read8(f);			

			if (note != 0x3f) {
				uint32 b = hio_read32b(f);
				event->note = note?note+24:0;
				event->ins = b >> 24;
				event->fxt = (b & 0xf00000) >> 20;
				event->f2t = (b & 0x0f0000) >> 16;
				event->fxp = (b & 0xff00) >> 8;
				event->f2p = b & 0xff;
				fix_effect (&event->fxt, &event->fxp);
				fix_effect (&event->f2t, &event->f2p);
				//	printf ("#");
			} //else 
			//	printf (".");
		}
		if (V(0) && !(i % mod->chn))
			report (".");
	}
	reportv(ctx, 0, "\n");

	free(seqbuf);
	free(transbuf);

	/*
	 * Instruments
	 */

	for (i = 0; i < mod->ins; i++) {
		uint8 buf[22];
		int vol, fspd, wavelen, flow, vibdel, hclen, hc;
		int vibdep, vibspd, sqmin, sqmax, sqspd, fmax, plen, pspd;
		int Alen, Avol, Dlen, Dvol, Slen, Rlen, Rvol;
                mod->xxi[i].sub = calloc(sizeof (struct xmp_subinstrument), 1);

		hio_read(buf, 22, 1, f);

		vol = buf[0];		/* Master volume (0 to 64) */
		fspd = ((buf[1] >> 3) & 0x1f) | ((buf[12] >> 2) & 0x20);
					/* Filter speed */
		wavelen = buf[1] & 7;	/* Wave length */

		Alen = buf[2];		/* attack length, 1 to 255 */
		Avol = buf[3];		/* attack volume, 0 to 64 */
		Dlen = buf[4];		/* decay length, 1 to 255 */
		Dvol = buf[5];		/* decay volume, 0 to 64 */
		Slen = buf[6];		/* sustain length, 1 to 255 */
		Rlen = buf[7];		/* release length, 1 to 255 */
		Rvol = buf[8];		/* release volume, 0 to 64 */

		flow = buf[12] & 0x7f;	/* filter modulation lower limit */
		vibdel = buf[13];	/* vibrato delay */
		hclen = (buf[14] >> 4) & 0x07;
					/* Hardcut length */
		hc = buf[14] & 0x80 ? 1 : 0;
					/* Hardcut release */
		vibdep = buf[14] & 15;	/* vibrato depth, 0 to 15 */
		vibspd = buf[15];	/* vibrato speed, 0 to 63 */
		sqmin = buf[16];	/* square modulation lower limit */
		sqmax = buf[17];	/* square modulation upper limit */
		sqspd = buf[18];	/* square modulation speed */
		fmax = buf[19] & 0x3f;	/* filter modulation upper limit */
		pspd = buf[20];		/* playlist default speed */	
		plen = buf[21];		/* playlist length */

		if (!pspd)
			pspd=1;
		int poff = 0;

		D_(D_WARN "I: %02x plen %02x pspd %02x vibdep=%d vibspd=%d sqmin=%d sqmax=%d", i, plen, pspd, vibdep, vibspd, sqmin, sqmax);
		int j;
		int wave=0;

		mod->xxi[i].fei.flg = XMP_ENVELOPE_ON; /* | XMP_ENVELOPE_LOOP;*/
		mod->xxi[i].fei.npt = plen*2;
		mod->xxfe[i] = calloc (4, mod->xxi[i].fei.npt);

		int note=0;
		int jump = -1;
		int dosq = -1;

		for (j = 0; j < plen; j++) {
			uint8 tmp[5];
			hio_read (tmp, 1, 5, f);

			int fx1 = tmp[0] & 15;
			int fx2 = (tmp[1] >> 3) & 15;

			/* non-zero waveform means change to wf[waveform-1] */
			/* 0: triangle
			   1: sawtooth
			   2: square
			   3: white noise
			*/

			int fixed = (tmp[2] >> 6) & 0x01;

			if (tmp[2] & 0x3f) {
				note = tmp[2] & 0x3f;
				if (fixed)
					note-=60;
			}

			if (fx1 == 15)
				pspd = tmp[3];
			else if (fx2 == 15)
				pspd = tmp[4];

			mod->xxfe[i][j*4] = poff;
			mod->xxfe[i][j*4+1] = note * 100;
			poff += pspd;
			mod->xxfe[i][j*4+2] = poff;
			mod->xxfe[i][j*4+3] = note * 100;

			if (jump >= 0) 
				continue;

			if ((fx1 == 4 && tmp[3] & 0xf) ||
			    (fx2 == 4 && tmp[4] & 0xf))
				dosq = sqmax;

			if (fx1 == 3)
				dosq = tmp[3] & 0x3f;
			else if (fx2 == 3)
				dosq = tmp[4] & 0x3f;

			if ((tmp[1] & 7))
				wave = tmp[1] & 7;
			
			if (fx1 == 5)
				jump = tmp[3];
			else if (fx2 == 5)
				jump = tmp[4];
			
			if (jump >= 0) {
				printf ("jump %d-%d\n", jump,j);
				mod->xxi[i].fei.flg |= XMP_ENVELOPE_LOOP;
				mod->xxi[i].fei.lps = jump*2;
				mod->xxi[i].fei.lpe = j*2+1;
			}

			D_(D_INFO "[%d W:%x 1:%x%02x 2:%x%02x n:%02x]", j, tmp[1] &7, tmp[0]&15, tmp[3], (tmp[1]>>3)&15, tmp[4], tmp[2]);
		}

		if (!wave)
			wave = 2;
		else if (dosq >= 0)
			wave = ((dosq>>2)&15)+4;
		else
			wave--;

		D_(D_INFO "I: %02x V: %02x A: %02x %02x D: %02x %02x S:  %02x R: %02x %02x wave %02x",
			i, vol, Alen, Avol, Dlen, Dvol, Slen, Rlen, Rvol, wave);
		mod->xxi[i].aei.flg = XMP_ENVELOPE_ON;
		mod->xxi[i].aei.npt = 5;
		mod->xxae[i] = calloc (4, mod->xxi[i].aei.npt);
		mod->xxae[i][0] = 0;
		mod->xxae[i][1] = vol;
		mod->xxae[i][2] = Alen; /* these are *not* multiplied by pspd */
		mod->xxae[i][3] = Avol;
		mod->xxae[i][4] = (Alen+Dlen);
		mod->xxae[i][5] = Dvol;
		mod->xxae[i][6] = (Alen+Dlen+Slen);
		mod->xxae[i][7] = Dvol;
		mod->xxae[i][8] = (Alen+Dlen+Slen+Rlen);
		mod->xxae[i][9] = Rvol;

		mod->xxi[i].sub[0].vol = 64;
		mod->xxi[i].sub[0].sid = wave;
		mod->xxi[i].sub[0].pan = 128;
		mod->xxi[i].sub[0].xpo = (3-wavelen) * 12 - 1;
		/*mod->xxi[i].sub[0].vde = vibdep;
		  mod->xxi[i].sub[0].vra = vibspd; */
		mod->xxi[i].nsm = 1;
        }

#define LEN 64
	int8 b[16384];

	for (i=0; i<20; i++) {
		mod->xxs[i].len = LEN;
		mod->xxs[i].lps = 0;
		mod->xxs[i].lpe = LEN;
		mod->xxs[i].flg |= XMP_SAMPLE_LOOP;

		switch (i) {
		case 0:
			hvl_GenTriangle (b, LEN);
			break;
		case 1:
			hvl_GenSawtooth (b, LEN);
			break;
		case 2:
			memset (b, 0x80, LEN/2);
			memset (b+LEN/2, 0x7f, LEN/2);
			break;
		case 3:
			mod->xxs[i].len = mod->xxs[i].lpe = 16384;
			hvl_GenWhiteNoise (b, 16384);
			break;
		default:
			memset (b, 0x7f, LEN);
			memset (b, 0x80, LEN*(20-i)/32);
			break;
		}

		load_sample(m, NULL, SAMPLE_FLAG_NOLOAD, &mod->xxs[i], (char *)b);
	}


	{
		int len, i;
		uint8 *namebuf, *nameptr;

		hio_seek (f, 0, SEEK_END);
		len = hio_tell(f) - title_offset;
		hio_seek (f, title_offset, SEEK_SET);

		nameptr = namebuf = malloc (len+1);
		hio_read (namebuf, 1, len, f);
		namebuf[len]=0;

		copy_adjust ((uint8 *)mod->name, namebuf, 32);
		mod->name[31]=0;
//		printf ("len=%d, name=%s\n", len, mod->name);
		
		for (i=0; nameptr < namebuf+len && i < mod->ins; i++) {
			nameptr += strlen((char *)nameptr)+1;
			instrument_name(mod, i, nameptr, 32);

			printf ("%02x: %s\n", i, nameptr);
		}

		free (namebuf);
	}

	for (i = 0; i < mod->chn; i++)
                mod->xxc[i].pan = ((i&3)%3) ? 128+stereo*31 : 128-stereo*31;


/*	m->quirk |= XMP_CTL_VBLANK;*/
/*	m->quirk |= QUIRK_UNISLD;*/
	return 0;
}


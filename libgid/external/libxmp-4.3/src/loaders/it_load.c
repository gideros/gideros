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

#ifndef LIBXMP_CORE_DISABLE_IT

#include <time.h>
#include "loader.h"
#include "it.h"
#include "period.h"

#define MAGIC_IMPM	MAGIC4('I','M','P','M')
#define MAGIC_IMPS	MAGIC4('I','M','P','S')


static int it_test (HIO_HANDLE *, char *, const int);
static int it_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader it_loader = {
    "Impulse Tracker",
    it_test,
    it_load
};

#ifdef WIN32
struct tm *localtime_r(const time_t *timep, struct tm *result)
{
    /* Note: Win32 localtime() is thread-safe */
    memcpy(result, localtime(timep), sizeof(struct tm));
    return result;
}
#endif

static int it_test(HIO_HANDLE *f, char *t, const int start)
{
    if (hio_read32b(f) != MAGIC_IMPM)
	return -1;

    read_title(f, t, 26);

    return 0;
}


#define	FX_NONE	0xff
#define FX_XTND 0xfe
#define L_CHANNELS 64


static const uint8 fx[] = {
	/*   */ FX_NONE,
	/* A */ FX_S3M_SPEED,
	/* B */ FX_JUMP,
	/* C */ FX_BREAK,
	/* D */ FX_VOLSLIDE,
	/* E */ FX_PORTA_DN,
	/* F */ FX_PORTA_UP,
	/* G */ FX_TONEPORTA,
	/* H */ FX_VIBRATO,
	/* I */ FX_TREMOR,
	/* J */ FX_S3M_ARPEGGIO,
	/* K */ FX_VIBRA_VSLIDE,
	/* L */ FX_TONE_VSLIDE,
	/* M */ FX_TRK_VOL,
	/* N */ FX_TRK_VSLIDE,
	/* O */ FX_OFFSET,
	/* P */ FX_IT_PANSLIDE,
	/* Q */ FX_MULTI_RETRIG,
	/* R */ FX_TREMOLO,
	/* S */ FX_XTND,
	/* T */ FX_IT_BPM,
	/* U */ FX_FINE_VIBRATO,
	/* V */ FX_GLOBALVOL,
	/* W */ FX_GVOL_SLIDE,
	/* X */ FX_SETPAN,
	/* Y */ FX_PANBRELLO,
	/* Z */ FX_FLT_CUTOFF
};


int itsex_decompress8 (HIO_HANDLE *, void *, int, int);
int itsex_decompress16 (HIO_HANDLE *, void *, int, int);


static void xlat_fx(int c, struct xmp_event *e, uint8 *last_fxp, int new_fx)
{
    uint8 h = MSN(e->fxp), l = LSN(e->fxp);

    switch (e->fxt = fx[e->fxt]) {
    case FX_XTND:		/* Extended effect */
	e->fxt = FX_EXTENDED;

	if (h == 0 && e->fxp == 0) {
	    e->fxp = last_fxp[c];
	    h = MSN(e->fxp);
	    l = LSN(e->fxp);
	} else {
	    last_fxp[c] = e->fxp;
	}

	switch (h) {
	case 0x1:		/* Glissando */
	    e->fxp = 0x30 | l;
	    break;
	case 0x2:		/* Finetune */
	    e->fxp = 0x50 | l;
	    break;
	case 0x3:		/* Vibrato wave */
	    e->fxp = 0x40 | l;
	    break;
	case 0x4:		/* Tremolo wave */
	    e->fxp = 0x70 | l;
	    break;
	case 0x5:		/* Panbrello wave */
	    if (l <= 3) {
	    	e->fxt = FX_PANBRELLO_WF;
	    	e->fxp = l;
	    } else {
		e->fxt = e->fxp = 0;
	    }
	    break;
	case 0x6:		/* Pattern delay */
	    e->fxp = 0xe0 | l;
	    break;
	case 0x7:		/* Instrument functions */
	    e->fxt = FX_IT_INSTFUNC;
	    e->fxp &= 0x0f;
	    break;
	case 0x8:		/* Set pan position */
	    e->fxt = FX_SETPAN;
	    e->fxp = l << 4;
	    break;
	case 0x9:		/* 0x91 = set surround */
            e->fxt = FX_SURROUND;
	    e->fxp = l;
	    break;
	case 0xa:		/* High offset */
            e->fxt = FX_HIOFFSET;
            e->fxp = l;
            break;
	case 0xb:		/* Pattern loop */
	    e->fxp = 0x60 | l;
	    break;
	case 0xc:		/* Note cut */
	case 0xd:		/* Note delay */
	    if ((e->fxp = l) == 0)
		e->fxp++;	/* SD0 and SC0 become SD1 and SC1 */
	    e->fxp |= h << 4;
	    break;
	case 0xe:		/* Pattern row delay */
	    e->fxt = FX_IT_ROWDELAY;
	    e->fxp = l;
	    break;
	default:
	    e->fxt = e->fxp = 0;
	}
	break;
    case FX_FLT_CUTOFF:
	if (e->fxp > 0x7f && e->fxp < 0x90) {	/* Resonance */
	    e->fxt = FX_FLT_RESN;
	    e->fxp = (e->fxp - 0x80) * 16;
	} else {		/* Cutoff */
	    e->fxp *= 2;
	}
	break;
    case FX_TREMOR:
	if (!new_fx && e->fxp != 0) {
	   e->fxp = ((MSN(e->fxp) + 1) << 4) | (LSN(e->fxp) + 1);
	}
	break;
    case FX_GLOBALVOL:
	if (e->fxp > 0x80) {	/* See storlek test 16 */
		e->fxt = e->fxp = 0;
	}
	break;
    case FX_NONE:		/* No effect */
	e->fxt = e->fxp = 0;
	break;
    }

    /* Impulse Tracker ignores the portamento command if there is an
     * portamento command next to an offset command
     */
    if (e->fxt == FX_OFFSET && e->f2t == FX_TONEPORTA) {
        e->f2t = e->f2p = 0;
    }
}


static void xlat_volfx(struct xmp_event *event)
{
    int b;

    b = event->vol;
    event->vol = 0;

    if (b <= 0x40) {
	event->vol = b + 1;
    } else if (b >= 65 && b <= 74) {	/* A */
	event->f2t = FX_F_VSLIDE_UP_2;
	event->f2p = b - 65;
    } else if (b >= 75 && b <= 84) {	/* B */
	event->f2t = FX_F_VSLIDE_DN_2;
	event->f2p = b - 75;
    } else if (b >= 85 && b <= 94) {	/* C */
	event->f2t = FX_VSLIDE_UP_2;
	event->f2p = b - 85;
    } else if (b >= 95 && b <= 104) {	/* D */
	event->f2t = FX_VSLIDE_DN_2;
	event->f2p = b - 95;
    } else if (b >= 105 && b <= 114) {	/* E */
	event->f2t = FX_PORTA_DN;
	event->f2p = (b - 105) << 2;
    } else if (b >= 115 && b <= 124) {	/* F */
	event->f2t = FX_PORTA_UP;
	event->f2p = (b - 115) << 2;
    } else if (b >= 128 && b <= 192) {	/* pan */
	if (b == 192)
	    b = 191;
	event->f2t = FX_SETPAN;
	event->f2p = (b - 128) << 2;
    } else if (b >= 193 && b <= 202) {	/* G */
	event->f2t = FX_TONEPORTA;
	event->f2p = (b - 193) << 2;
    } else if (b >= 203 && b <= 212) {	/* H */
	event->f2t = FX_VIBRATO;
	event->f2p = b - 203;
    }
}


static void fix_name(uint8 *s, int l)
{
    int i;

    /* IT names can have 0 at start of data, replace with space */
    for (l--, i = 0; i < l; i++) {
	if (s[i] == 0)
	    s[i] = ' ';
    }
    for (i--; i >= 0 && s[i] == ' '; i--) {
	if (s[i] == ' ')
	    s[i] = 0;
    }
}


static void read_envelope(struct xmp_envelope *ei, struct it_envelope *env, HIO_HANDLE *f)
{
    int j;

    env->flg = hio_read8(f);
    env->num = hio_read8(f);

    /* Sanity check */
    if (env->num >= XMP_MAX_ENV_POINTS) {
	env->flg = 0;
	env->num = 0;
	return;
    }

    env->lpb = hio_read8(f);
    env->lpe = hio_read8(f);
    env->slb = hio_read8(f);
    env->sle = hio_read8(f);

    for (j = 0; j < 25; j++) {
    	env->node[j].y = hio_read8(f);
    	env->node[j].x = hio_read16l(f);
    }

    env->unused = hio_read8(f);

    ei->flg = env->flg & IT_ENV_ON ? XMP_ENVELOPE_ON : 0;
    ei->flg |= env->flg & IT_ENV_LOOP ? XMP_ENVELOPE_LOOP : 0;
    ei->flg |= env->flg & IT_ENV_SLOOP ? (XMP_ENVELOPE_SUS|XMP_ENVELOPE_SLOOP) : 0;
    ei->flg |= env->flg & IT_ENV_CARRY ? XMP_ENVELOPE_CARRY : 0;
    ei->npt = env->num;
    ei->sus = env->slb;
    ei->sue = env->sle;
    ei->lps = env->lpb;
    ei->lpe = env->lpe;

    if (ei->npt > 0 && ei->npt < 25 /* XMP_MAX_ENV_POINTS */) {
    	for (j = 0; j < ei->npt; j++) {
    		ei->data[j * 2] = env->node[j].x;
    		ei->data[j * 2 + 1] = env->node[j].y;
    	}
    } else {
    	ei->flg &= ~XMP_ENVELOPE_ON;
    }
}

static int it_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
    struct xmp_module *mod = &m->mod;
    int r, c, i, j, k, pat_len;
    struct xmp_event *event, dummy, lastevent[L_CHANNELS];
    struct it_file_header ifh;
    struct it_instrument1_header i1h;
    struct it_instrument2_header i2h;
    struct it_sample_header ish;
    struct it_envelope env;
    uint8 b, mask[L_CHANNELS];
    int max_ch;
    int inst_map[120], inst_rmap[XMP_MAX_KEYS];
#ifndef LIBXMP_CORE_PLAYER
    char tracker_name[40];
#endif
    uint32 *pp_ins;		/* Pointers to instruments */
    uint32 *pp_smp;		/* Pointers to samples */
    uint32 *pp_pat;		/* Pointers to patterns */
    uint8 last_fxp[64];
    int dca2nna[] = { 0, 2, 3 };
    int new_fx, sample_mode;

    LOAD_INIT();

    /* Load and convert header */
    hio_read32b(f);		/* magic */

    hio_read(&ifh.name, 26, 1, f);
    ifh.hilite_min = hio_read8(f);
    ifh.hilite_maj = hio_read8(f);

    ifh.ordnum = hio_read16l(f);
    ifh.insnum = hio_read16l(f);
    ifh.smpnum = hio_read16l(f);
    ifh.patnum = hio_read16l(f);

    ifh.cwt = hio_read16l(f);
    ifh.cmwt = hio_read16l(f);
    ifh.flags = hio_read16l(f);
    ifh.special = hio_read16l(f);

    ifh.gv = hio_read8(f);
    ifh.mv = hio_read8(f);
    ifh.is = hio_read8(f);
    ifh.it = hio_read8(f);
    ifh.sep = hio_read8(f);
    ifh.pwd = hio_read8(f);

    /* Sanity check */
    if (ifh.gv > 0x80 || ifh.mv > 0x80) {
        goto err;
    }

    ifh.msglen = hio_read16l(f);
    ifh.msgofs = hio_read32l(f);
    ifh.rsvd = hio_read32l(f);

    hio_read(&ifh.chpan, 64, 1, f);
    hio_read(&ifh.chvol, 64, 1, f);

    strncpy(mod->name, (char *)ifh.name, XMP_NAME_SIZE);
    mod->len = ifh.ordnum;
    mod->ins = ifh.insnum;
    mod->smp = ifh.smpnum;
    mod->pat = ifh.patnum;

    memset(lastevent, 0, L_CHANNELS * sizeof (struct xmp_event));
    memset(&dummy, 0, sizeof (struct xmp_event));

    /* Sanity check */
    if (mod->ins > 255 || mod->smp > 255 || mod->pat > 255) {
	goto err;
    }

    if (mod->ins) {
        pp_ins = calloc(4, mod->ins);
        if (pp_ins == NULL)
	    goto err;
    } else {
	pp_ins = NULL;
    }

    pp_smp = calloc(4, mod->smp);
    if (pp_smp == NULL)
	goto err2;

    pp_pat = calloc(4, mod->pat);
    if (pp_pat == NULL)
	goto err3;

    mod->spd = ifh.is;
    mod->bpm = ifh.it;

    sample_mode = ~ifh.flags & IT_USE_INST;

    if (ifh.flags & IT_LINEAR_FREQ) {
        m->quirk |= QUIRK_LINEAR;
    }

    if (!sample_mode && ifh.cmwt >= 0x200) {
        m->quirk |= QUIRK_INSVOL;
    }

    for (i = 0; i < 64; i++) {
	struct xmp_channel *xxc = &mod->xxc[i];

	if (ifh.chpan[i] == 100) {	/* Surround -> center */
	    mod->xxc[i].flg |= XMP_CHANNEL_SURROUND;
        }

	if (ifh.chpan[i] & 0x80) {	/* Channel mute */
	    ifh.chvol[i] = 0;
	    mod->xxc[i].flg |= XMP_CHANNEL_MUTE;
	}

	if (ifh.flags & IT_STEREO) {
	    xxc->pan = (int)ifh.chpan[i] * 0x80 >> 5;
	    if (xxc->pan > 0xff)
		xxc->pan = 0xff;
	} else {
	    xxc->pan = 0x80;
	}

	xxc->vol = ifh.chvol[i];
    }
    if (mod->len <= XMP_MAX_MOD_LENGTH) {
    	hio_read(mod->xxo, 1, mod->len, f);
    } else {
    	hio_read(mod->xxo, 1, XMP_MAX_MOD_LENGTH, f);
    	hio_seek(f, mod->len - XMP_MAX_MOD_LENGTH, SEEK_CUR);
    	mod->len = XMP_MAX_MOD_LENGTH;
    }

    new_fx = ifh.flags & IT_OLD_FX ? 0 : 1;

    for (i = 0; i < mod->ins; i++)
	pp_ins[i] = hio_read32l(f);
    for (i = 0; i < mod->smp; i++)
	pp_smp[i] = hio_read32l(f);
    for (i = 0; i < mod->pat; i++)
	pp_pat[i] = hio_read32l(f);

    m->c4rate = C4_NTSC_RATE;

#ifndef LIBXMP_CORE_PLAYER
    /* Identify tracker */

    switch (ifh.cwt >> 8) {
    case 0x00:
	strcpy(tracker_name, "unmo3");
	break;
    case 0x01:
    case 0x02:		/* test from Schism Tracker sources */
	if (ifh.cmwt == 0x0200 && ifh.cwt == 0x0214
		&& ifh.flags == 9 && ifh.special == 0
		&& ifh.hilite_maj == 0 && ifh.hilite_min == 0
		&& ifh.insnum == 0 && ifh.patnum + 1 == ifh.ordnum
		&& ifh.gv == 128 && ifh.mv == 100 && ifh.is == 1
		&& ifh.sep == 128 && ifh.pwd == 0
		&& ifh.msglen == 0 && ifh.msgofs == 0 && ifh.rsvd == 0)
	{
                strcpy(tracker_name, "OpenSPC conversion");
	} else if (ifh.cmwt == 0x0200 && ifh.cwt == 0x0217) {
	    strcpy(tracker_name, "ModPlug Tracker 1.16");
	    /* ModPlug Tracker files aren't really IMPM 2.00 */
	    ifh.cmwt = sample_mode ? 0x100 : 0x214;	
	} else if (ifh.cwt == 0x0216) {
	    strcpy(tracker_name, "Impulse Tracker 2.14v3");
	} else if (ifh.cwt == 0x0217) {
	    strcpy(tracker_name, "Impulse Tracker 2.14v5");
	} else if (ifh.cwt == 0x0214 && !memcmp(&ifh.rsvd, "CHBI", 4)) {
	    strcpy(tracker_name, "Chibi Tracker");
	} else {
	    snprintf(tracker_name, 40, "Impulse Tracker %d.%02x",
			(ifh.cwt & 0x0f00) >> 8, ifh.cwt & 0xff);
	}
	break;
    case 0x08:
    case 0x7f:
	if (ifh.cwt == 0x0888) {
	    strcpy(tracker_name, "OpenMPT 1.17");
	} else if (ifh.cwt == 0x7fff) {
	    strcpy(tracker_name, "munch.py");
	} else {
	    snprintf(tracker_name, 40, "unknown (%04x)", ifh.cwt);
	}
	break;
    default:
	switch (ifh.cwt >> 12) {
	case 0x1: {
	    uint16 cwtv = ifh.cwt & 0x0fff;
	    struct tm version;
	    time_t version_sec;

	    if (cwtv > 0x50) {
		version_sec = ((cwtv - 0x050) * 86400) + 1254355200;
		if (localtime_r(&version_sec, &version)) {
		    snprintf(tracker_name, 40, "Schism Tracker %04d-%02d-%02d",
				version.tm_year + 1900, version.tm_mon + 1,
				version.tm_mday);
                }
	    } else {
	    	snprintf(tracker_name, 40, "Schism Tracker 0.%x", cwtv);
	    }
	    break; }
	case 0x5:
	    snprintf(tracker_name, 40, "OpenMPT %d.%02x",
			(ifh.cwt & 0x0f00) >> 8, ifh.cwt & 0xff);
	    if (memcmp(&ifh.rsvd, "OMPT", 4))
		strncat(tracker_name, " (compat.)", 40);
	    break;
	case 0x06:
	    snprintf(tracker_name, 40, "BeRoTracker %d.%02x",
			(ifh.cwt & 0x0f00) >> 8, ifh.cwt & 0xff);
	    break;
	default:
	    snprintf(tracker_name, 40, "unknown (%04x)", ifh.cwt);
	}
    }

    set_type(m, "%s IT %d.%02x", tracker_name,
			ifh.cmwt >> 8, ifh.cmwt & 0xff);
#else
    set_type(m, "Impulse Tracker");
#endif

    MODULE_INFO();

    D_(D_INFO "Instrument/FX mode: %s/%s",
			sample_mode ? "sample" : ifh.cmwt >= 0x200 ?
			"new" : "old", ifh.flags & IT_OLD_FX ? "old" : "IT");

    if (sample_mode)
	mod->ins = mod->smp;

    if (instrument_init(mod) < 0)
	goto err4;

    D_(D_INFO "Instruments: %d", mod->ins);

    for (i = 0; i < mod->ins; i++) {
	/*
	 * IT files can have three different instrument types: 'New'
	 * instruments, 'old' instruments or just samples. We need a
	 * different loader for each of them.
	 */

	struct xmp_instrument *xxi = &mod->xxi[i];

	if (!sample_mode && ifh.cmwt >= 0x200) {
	    /* New instrument format */
	    if (hio_seek(f, start + pp_ins[i], SEEK_SET) != 0)
	    	goto err4;

	    i2h.magic = hio_read32b(f);
	    hio_read(&i2h.dosname, 12, 1, f);
	    i2h.zero = hio_read8(f);
	    i2h.nna = hio_read8(f);
	    i2h.dct = hio_read8(f);
	    i2h.dca = hio_read8(f);
	    i2h.fadeout = hio_read16l(f);

	    i2h.pps = hio_read8(f);
	    i2h.ppc = hio_read8(f);
	    i2h.gbv = hio_read8(f);
	    i2h.dfp = hio_read8(f);
	    i2h.rv = hio_read8(f);
	    i2h.rp = hio_read8(f);
	    i2h.trkvers = hio_read16l(f);

	    i2h.nos = hio_read8(f);
	    i2h.rsvd1 = hio_read8(f);

            if (hio_read(&i2h.name, 1, 26, f) != 26) {
                goto err4;
            }

	    fix_name(i2h.name, 26);

	    i2h.ifc = hio_read8(f);
	    i2h.ifr = hio_read8(f);
	    i2h.mch = hio_read8(f);
	    i2h.mpr = hio_read8(f);
	    i2h.mbnk = hio_read16l(f);

            if (hio_read(&i2h.keys, 1, 240, f) != 240) {
                goto err4; 
            }

	    copy_adjust(xxi->name, i2h.name, 25);
	    xxi->rls = i2h.fadeout << 6;

	    /* Envelopes */

	    read_envelope(&xxi->aei, &env, f);
	    read_envelope(&xxi->pei, &env, f);
	    read_envelope(&xxi->fei, &env, f);
	    
	    if (xxi->pei.flg & XMP_ENVELOPE_ON) {
		for (j = 0; j < xxi->pei.npt; j++)
		    xxi->pei.data[j * 2 + 1] += 32;
	    }

	    if (xxi->aei.flg & XMP_ENVELOPE_ON && xxi->aei.npt == 0)
		xxi->aei.npt = 1;
	    if (xxi->pei.flg & XMP_ENVELOPE_ON && xxi->pei.npt == 0)
		xxi->pei.npt = 1;
	    if (xxi->fei.flg & XMP_ENVELOPE_ON && xxi->fei.npt == 0)
		xxi->fei.npt = 1;

	    if (env.flg & IT_ENV_FILTER) {
		xxi->fei.flg |= XMP_ENVELOPE_FLT;
		for (j = 0; j < env.num; j++) {
		    xxi->fei.data[j * 2 + 1] += 32;
		    xxi->fei.data[j * 2 + 1] *= 4;
		}
	    } else {
		/* Pitch envelope is *50 to get fine interpolation */
		for (j = 0; j < env.num; j++)
		    xxi->fei.data[j * 2 + 1] *= 50;
	    }

	    /* See how many different instruments we have */
	    for (j = 0; j < 120; j++)
		inst_map[j] = -1;

	    for (k = j = 0; j < 120; j++) {
		c = i2h.keys[j * 2 + 1] - 1;
		if (c < 0 || c >= 120) {
		    xxi->map[j].ins = 0xff;	/* No sample */
		    xxi->map[j].xpo = 0;
		    continue;
		}
		if (inst_map[c] == -1) {
		    inst_map[c] = k;
		    inst_rmap[k] = c;
		    k++;
		}
		xxi->map[j].ins = inst_map[c];
		xxi->map[j].xpo = i2h.keys[j * 2] - j;
	    }

	    xxi->nsm = k;
	    xxi->vol = i2h.gbv >> 1;

	    if (k) {
		xxi->sub = calloc(sizeof (struct xmp_subinstrument), k);
		if (xxi->sub == NULL)
		    goto err4;

		for (j = 0; j < k; j++) {
		    struct xmp_subinstrument *sub = &xxi->sub[j];

		    sub->sid = inst_rmap[j];
		    sub->nna = i2h.nna;
		    sub->dct = i2h.dct;
		    sub->dca = dca2nna[i2h.dca & 0x03];
		    sub->pan = i2h.dfp & 0x80 ? -1 : i2h.dfp * 4;
		    sub->ifc = i2h.ifc;
		    sub->ifr = i2h.ifr;
                    sub->rvv = ((int)i2h.rp << 8) | i2h.rv;
	        }
	    }

	    D_(D_INFO "[%2X] %-26.26s %d %d %d %4d %4d  %2x "
			"%02x %c%c%c %3d %02x %02x",
		i, i2h.name,
		i2h.nna, i2h.dct, i2h.dca,
		i2h.fadeout,
		i2h.gbv,
		i2h.dfp & 0x80 ? 0x80 : i2h.dfp * 4,
		i2h.rv,
		xxi->aei.flg & XMP_ENVELOPE_ON ? 'V' : '-',
		xxi->pei.flg & XMP_ENVELOPE_ON ? 'P' : '-',
		env.flg & 0x01 ? env.flg & 0x80 ? 'F' : 'P' : '-',
		xxi->nsm,
		i2h.ifc,
		i2h.ifr
	    );

	} else if (!sample_mode) {
	    /* Old instrument format */
	    hio_seek(f, start + pp_ins[i], SEEK_SET);

	    i1h.magic = hio_read32b(f);
	    hio_read(&i1h.dosname, 12, 1, f);

	    i1h.zero = hio_read8(f);
	    i1h.flags = hio_read8(f);
	    i1h.vls = hio_read8(f);
	    i1h.vle = hio_read8(f);
	    i1h.sls = hio_read8(f);
	    i1h.sle = hio_read8(f);
	    i1h.rsvd1 = hio_read16l(f);
	    i1h.fadeout = hio_read16l(f);

	    i1h.nna = hio_read8(f);
	    i1h.dnc = hio_read8(f);
	    i1h.trkvers = hio_read16l(f);
	    i1h.nos = hio_read8(f);
	    i1h.rsvd2 = hio_read8(f);

	    if (hio_read(&i1h.name, 1, 26, f) != 26) {
                goto err4;
            }

	    fix_name(i1h.name, 26);

	    if (hio_read(&i1h.rsvd3, 1, 6, f) != 6)
		goto err4;
	    if (hio_read(&i1h.keys, 1, 240, f) != 240)
		goto err4;
	    if (hio_read(&i1h.epoint, 1, 200, f) != 200)
		goto err4;
	    if (hio_read(&i1h.enode, 1, 50, f) != 50)
		goto err4;

	    copy_adjust(xxi->name, i1h.name, 25);

	    xxi->rls = i1h.fadeout << 7;

	    xxi->aei.flg = 0;
	    if (i1h.flags & IT_ENV_ON) {
		xxi->aei.flg |= XMP_ENVELOPE_ON;
	    }
	    if (i1h.flags & IT_ENV_LOOP) {
		xxi->aei.flg |= XMP_ENVELOPE_LOOP;
	    }
	    if (i1h.flags & IT_ENV_SLOOP) {
		xxi->aei.flg |= XMP_ENVELOPE_SUS | XMP_ENVELOPE_SLOOP;
	    }
	    if (i1h.flags & IT_ENV_CARRY) {
		xxi->aei.flg |= XMP_ENVELOPE_SUS | XMP_ENVELOPE_CARRY;
	    }
	    xxi->aei.lps = i1h.vls;
	    xxi->aei.lpe = i1h.vle;
	    xxi->aei.sus = i1h.sls;
	    xxi->aei.sue = i1h.sle;

	    for (k = 0; k < 25 && i1h.enode[k * 2] != 0xff; k++);

	    /* Sanity check */
	    if (k >= 25 || i1h.enode[k * 2] != 0xff) {
		goto err4;
	    }

	    for (xxi->aei.npt = k; k--; ) {
		xxi->aei.data[k * 2] = i1h.enode[k * 2];
		xxi->aei.data[k * 2 + 1] = i1h.enode[k * 2 + 1];
	    }
	    
	    /* See how many different instruments we have */
	    for (j = 0; j < 120; j++)
		inst_map[j] = -1;

	    for (k = j = 0; j < XMP_MAX_KEYS; j++) {
		c = j < 120 ? i1h.keys[j * 2 + 1] - 1 : -1;
		if (c < 0 || c >= 120) {
		    xxi->map[j].ins = 0;
		    xxi->map[j].xpo = 0;
		    continue;
		}
		if (inst_map[c] == -1) {
		    inst_map[c] = k;
		    inst_rmap[k] = c;
		    k++;
		}
		xxi->map[j].ins = inst_map[c];
		xxi->map[j].xpo = i1h.keys[j * 2] - j;
	    }

	    xxi->nsm = k;
	    xxi->vol = i2h.gbv >> 1;

	    if (k) {
		xxi->sub = calloc(sizeof (struct xmp_subinstrument), k);
 		if (xxi->sub == NULL)
		    goto err4;

		for (j = 0; j < k; j++) {
		    struct xmp_subinstrument *sub = &xxi->sub[j];

		    sub->sid = inst_rmap[j];
		    sub->nna = i1h.nna;
		    sub->dct = i1h.dnc ? XMP_INST_DCT_NOTE : XMP_INST_DCT_OFF;
		    sub->dca = XMP_INST_DCA_CUT;
		    sub->pan = 0x80;
	        }
	    }

	    D_(D_INFO "[%2X] %-26.26s %d %-4.4s %4d %2d %c%c%c %3d",
		i, i1h.name,
		i1h.nna,
		i1h.dnc ? "on" : "off",
		i1h.fadeout,
		xxi->aei.npt,
		xxi->aei.flg & XMP_ENVELOPE_ON ? 'V' : '-',
		xxi->aei.flg & XMP_ENVELOPE_LOOP ? 'L' : '-',
		xxi->aei.flg & XMP_ENVELOPE_SUS ? 'S' : '-',
		xxi->nsm
	    );
	}
    }

    D_(D_INFO "Stored Samples: %d", mod->smp);

    for (i = 0; i < mod->smp; i++) {
	struct xmp_sample *xxs = &mod->xxs[i];

	if (sample_mode) {
	    mod->xxi[i].sub = calloc(sizeof (struct xmp_subinstrument), 1);
	    if (mod->xxi[i].sub == NULL)
		goto err4;
	}

	hio_seek(f, start + pp_smp[i], SEEK_SET);

	ish.magic = hio_read32b(f);
	hio_read(&ish.dosname, 12, 1, f);
	ish.zero = hio_read8(f);
	ish.gvl = hio_read8(f);
	ish.flags = hio_read8(f);
	ish.vol = hio_read8(f);

	if (hio_read(&ish.name, 1, 26, f) != 26) {
	    goto err4;
	}

	fix_name(ish.name, 26);

	ish.convert = hio_read8(f);
	ish.dfp = hio_read8(f);
	ish.length = hio_read32l(f);
	ish.loopbeg = hio_read32l(f);
	ish.loopend = hio_read32l(f);
	ish.c5spd = hio_read32l(f);
	ish.sloopbeg = hio_read32l(f);
	ish.sloopend = hio_read32l(f);
	ish.sample_ptr = hio_read32l(f);

	ish.vis = hio_read8(f);
	ish.vid = hio_read8(f);
	ish.vir = hio_read8(f);
	ish.vit = hio_read8(f);

	/* Changed to continue to allow use-brdg.it and use-funk.it to
	 * load correctly (both IT 2.04)
	 */
	if (ish.magic != MAGIC_IMPS)
	    continue;
	
	if (ish.flags & IT_SMP_16BIT) {
	    xxs->flg = XMP_SAMPLE_16BIT;
	}
	xxs->len = ish.length;

	/* Sanity check */
	if (xxs->len > MAX_SAMPLE_SIZE) {
	    goto err4;
	}

	xxs->lps = ish.loopbeg;
	xxs->lpe = ish.loopend;

	xxs->flg |= ish.flags & IT_SMP_LOOP ? XMP_SAMPLE_LOOP : 0;
	xxs->flg |= ish.flags & IT_SMP_BLOOP ? XMP_SAMPLE_LOOP_BIDIR : 0;

	if (sample_mode) {
	    /* Create an instrument for each sample */
	    mod->xxi[i].sub[0].vol = ish.vol;
	    mod->xxi[i].sub[0].pan = 0x80;
	    mod->xxi[i].sub[0].sid = i;
	    mod->xxi[i].nsm = !!(xxs->len);
	    instrument_name(mod, i, ish.name, 25);
	} else {
	    copy_adjust(xxs->name, ish.name, 25);
	}

	D_(D_INFO "\n[%2X] %-26.26s %05x%c%05x %05x %05x %05x "
		    "%02x%02x %02x%02x %5d ",
		    i, sample_mode ? xxs->name : mod->xxi[i].name,
		    xxs->len,
		    ish.flags & IT_SMP_16BIT ? '+' : ' ',
		    MIN(xxs->lps, 0xfffff), MIN(xxs->lpe, 0xfffff),
		    MIN(ish.sloopbeg, 0xfffff), MIN(ish.sloopend, 0xfffff),
		    ish.flags, ish.convert,
		    ish.vol, ish.gvl, ish.c5spd);

	/* Convert C5SPD to relnote/finetune
	 *
	 * In IT we can have a sample associated with two or more
	 * instruments, but c5spd is a sample attribute -- so we must
	 * scan all xmp instruments to set the correct transposition
	 */
	
	for (j = 0; j < mod->ins; j++) {
	    for (k = 0; k < mod->xxi[j].nsm; k++) {
		struct xmp_subinstrument *sub = &mod->xxi[j].sub[k];
		if (sub->sid == i) {
		    sub->vol = ish.vol;
		    sub->gvl = ish.gvl;
		    sub->vra = ish.vis;	/* sample to sub-instrument vibrato */
		    sub->vde = ish.vid >> 1;
		    sub->vwf = ish.vit;
		    sub->vsw = (0xff - ish.vir) >> 1;

		    c2spd_to_note(ish.c5spd, &mod->xxi[j].sub[k].xpo, &mod->xxi[j].sub[k].fin);

                    /* Set sample pan (overrides subinstrument) */
                    if (ish.dfp & 0x80) {
                        sub->pan = (ish.dfp & 0x7f) * 4;
                    }
		}
	    }
	}

	if (ish.flags & IT_SMP_SAMPLE && xxs->len > 1) {
	    int cvt = 0;

	    if (0 != hio_seek(f, start + ish.sample_ptr, SEEK_SET))
	    	goto err4;

	    if (xxs->lpe > xxs->len || xxs->lps >= xxs->lpe)
	    	xxs->flg &= ~XMP_SAMPLE_LOOP;

	    if (~ish.convert & IT_CVT_SIGNED)
		cvt |= SAMPLE_FLAG_UNS;

	    /* compressed samples */
	    if (ish.flags & IT_SMP_COMP) {
		uint8 *buf;
	    	int ret;

		buf = calloc(1, xxs->len * 2);
		if (buf == NULL)
		    goto err4;

		if (ish.flags & IT_SMP_16BIT) {
		    itsex_decompress16(f, buf, xxs->len, 
					ish.convert & IT_CVT_DIFF);

#ifdef WORDS_BIGENDIAN
		    /* decompression generates native-endian samples, but
		     * we want little-endian */
		    cvt |= SAMPLE_FLAG_BIGEND;
#endif
		} else {
		    itsex_decompress8(f, buf, xxs->len, 
					ish.convert & IT_CVT_DIFF);
		}

		ret = load_sample(m, NULL, SAMPLE_FLAG_NOLOAD | cvt,
							&mod->xxs[i], buf);
		if (ret < 0) {
		    free(buf);
                    goto err4;
		}
		free (buf);
	    } else {
		if (load_sample(m, f, cvt, &mod->xxs[i], NULL) < 0)
                    goto err4;
	    }
	}
    }

    D_(D_INFO "Stored Patterns: %d", mod->pat);

    /* Effects in muted channels are processed, so scan patterns first to
     * see the real number of channels
     */
    max_ch = 0;
    for (i = 0; i < mod->pat; i++) {
	/* If the offset to a pattern is 0, the pattern is empty */
	if (pp_pat[i] == 0)
	    continue;

	hio_seek(f, start + pp_pat[i], SEEK_SET);
	pat_len = hio_read16l(f) /* - 4*/;
	hio_read16l(f);
	memset(mask, 0, L_CHANNELS);
	hio_read16l(f);
	hio_read16l(f);

	while (--pat_len >= 0) {
	    b = hio_read8(f);
	    if (b == 0)
		continue;

	    c = (b - 1) & 63;

	    if (c > max_ch)
		max_ch = c;

	    if (b & 0x80) {
		mask[c] = hio_read8(f);
		pat_len--;
	    }

	    if (mask[c] & 0x01) {
		hio_read8(f);
		pat_len--;
	    }
	    if (mask[c] & 0x02) {
		hio_read8(f);
		pat_len--;
	    }
	    if (mask[c] & 0x04) {
		hio_read8(f);
		pat_len--;
	    }
	    if (mask[c] & 0x08) {
		hio_read8(f);
		hio_read8(f);
		pat_len -= 2;
	    }
	}
    }

    /* Set the number of channels actually used
     */
    mod->chn = max_ch + 1;
    mod->trk = mod->pat * mod->chn;

    memset(last_fxp, 0, 64);

    if (pattern_init(mod) < 0)
	goto err4;

    /* Read patterns */
    for (i = 0; i < mod->pat; i++) {

	if (pattern_alloc(mod, i) < 0)
	    goto err4;

	r = 0;

	/* If the offset to a pattern is 0, the pattern is empty */
	if (pp_pat[i] == 0) {
	    mod->xxp[i]->rows = 64;
	    for (j = 0; j < mod->chn; j++) {
		int tnum = i * mod->chn + j;
		if (track_alloc(mod, tnum, 64) < 0)
		    goto err4;
		mod->xxp[i]->index[j] = tnum;
	    }
	    continue;
	}

	hio_seek(f, start + pp_pat[i], SEEK_SET);
	pat_len = hio_read16l(f) /* - 4*/;
	mod->xxp[i]->rows = hio_read16l(f);

	if (tracks_in_pattern_alloc(mod, i) < 0)
	    goto err4;

	memset(mask, 0, L_CHANNELS);
	hio_read16l(f);
	hio_read16l(f);

	while (--pat_len >= 0) {
	    b = hio_read8(f);
	    if (!b) {
		r++;
		continue;
	    }
	    c = (b - 1) & 63;

	    if (b & 0x80) {
		mask[c] = hio_read8(f);
		pat_len--;
	    }
	    /*
	     * WARNING: we IGNORE events in disabled channels. Disabled
	     * channels should be muted only, but we don't know the
	     * real number of channels before loading the patterns and
	     * we don't want to set it to 64 channels.
	     */
            if (c >= mod->chn || r >= mod->xxp[i]->rows) {
                event = &dummy;
            } else {
                event = &EVENT(i, c, r);
            }

	    if (mask[c] & 0x01) {
		b = hio_read8(f);

		/* From ittech.txt:
		 * Note ranges from 0->119 (C-0 -> B-9)
		 * 255 = note off, 254 = notecut
		 * Others = note fade (already programmed into IT's player
		 *                     but not available in the editor)
		 */
		switch (b) {
		case 0xff:	/* key off */
		    b = XMP_KEY_OFF;
		    break;
		case 0xfe:	/* cut */
		    b = XMP_KEY_CUT;
		    break;
		default:
		    if (b > 119)	/* fade */
			b = XMP_KEY_FADE;
		    else
                        b++;	/* note */
		}
		lastevent[c].note = event->note = b;
		pat_len--;
	    }
	    if (mask[c] & 0x02) {
		b = hio_read8(f);
		lastevent[c].ins = event->ins = b;
		pat_len--;
	    }
	    if (mask[c] & 0x04) {
		b = hio_read8(f);
		lastevent[c].vol = event->vol = b;
		xlat_volfx(event);
		pat_len--;
	    }
	    if (mask[c] & 0x08) {
		b = hio_read8(f);
		event->fxt = b;
		event->fxp = hio_read8(f);
		xlat_fx(c, event, last_fxp, new_fx);
		lastevent[c].fxt = event->fxt;
		lastevent[c].fxp = event->fxp;
		pat_len -= 2;
	    }
	    if (mask[c] & 0x10) {
		event->note = lastevent[c].note;
	    }
	    if (mask[c] & 0x20) {
		event->ins = lastevent[c].ins;
	    }
	    if (mask[c] & 0x40) {
		event->vol = lastevent[c].vol;
		xlat_volfx(event);
	    }
	    if (mask[c] & 0x80) {
		event->fxt = lastevent[c].fxt;
		event->fxp = lastevent[c].fxp;
	    }
	}
    }

    free(pp_pat);
    free(pp_smp);
    free(pp_ins);

    /* Song message */

    if (ifh.special & IT_HAS_MSG) {
	if ((m->comment = malloc(ifh.msglen + 1)) != NULL) {
	    hio_seek(f, start + ifh.msgofs, SEEK_SET);

	    D_(D_INFO "Message length : %d", ifh.msglen);

	    for (j = 0; j < ifh.msglen; j++) {
	        b = hio_read8(f);
	        if (b == '\r') {
		    b = '\n';
	        } else if ((b < 32 || b > 127) && b != '\n' && b != '\t') {
		    b = '.';
		}
	        m->comment[j] = b;
	    }
	    m->comment[j] = 0;
	}
    }

    /* Format quirks */

    m->quirk |= QUIRKS_IT;

    if (ifh.flags & IT_LINK_GXX) {
	m->quirk |= QUIRK_PRENV;
    } else {
	m->quirk |= QUIRK_UNISLD;
    } 

    if (new_fx) {
	m->quirk |= QUIRK_VIBHALF | QUIRK_VIBINV;
    } else {
	m->quirk &= ~QUIRK_VIBALL;
	m->quirk |= QUIRK_ITOLDFX;
    }

    if (sample_mode) {
	m->quirk &= ~(QUIRK_VIRTUAL | QUIRK_RSTCHN);
    }

    m->gvolbase = 0x80;
    m->gvol = ifh.gv;
    m->read_event_type = READ_EVENT_IT;

    return 0;

  err4:
    free(pp_pat);
  err3:
    free(pp_smp);
  err2:
    free(pp_ins);
  err:
    return -1;
}

#endif /* LIBXMP_CORE_DISABLE_IT */

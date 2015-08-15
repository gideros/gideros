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
 * Fri, 26 Jun 1998 17:45:59 +1000  Andrew Leahy <alf@cit.nepean.uws.edu.au>
 * Finally got it working on the DEC Alpha running DEC UNIX! In the pattern
 * reading loop I found I was getting "0" for (p-patbuf) and "0" for
 * xph.datasize, the next if statement (where it tries to read the patbuf)
 * would then cause a seg_fault.
 *
 * Sun Sep 27 12:07:12 EST 1998  Claudio Matsuoka <claudio@pos.inf.ufpr.br>
 * Extended Module 1.02 stores data in a different order, we must handle
 * this accordingly. MAX_SAMP used as a workaround to check the number of
 * samples recognized by the player.
 */

#include "loader.h"
#include "xm.h"


static int xm_test (HIO_HANDLE *, char *, const int);
static int xm_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader xm_loader = {
    "Fast Tracker II",
    xm_test,
    xm_load
};

static int xm_test(HIO_HANDLE *f, char *t, const int start)
{
    char buf[20];

    if (hio_read(buf, 1, 17, f) < 17)		/* ID text */
	return -1;

    if (memcmp(buf, "Extended Module: ", 17))
	return -1;

    read_title(f, t, 20);

    return 0;
}

static int load_patterns(struct module_data *m, int version, HIO_HANDLE *f)
{
    struct xmp_module *mod = &m->mod;
    struct xm_pattern_header xph;
    struct xmp_event *event;
    uint8 *patbuf, *pat, b;
    int i, j, r;

    mod->pat++;
    if (pattern_init(mod) < 0)
	return -1;

    D_(D_INFO "Stored patterns: %d", mod->pat - 1);

    /* Endianism fixed by Miodrag Vallat <miodrag@multimania.com>
     * Mon, 04 Jan 1999 11:17:20 +0100
     */
    for (i = 0; i < mod->pat - 1; i++) {
    	const int headsize = version > 0x0102 ? 9 : 8;

	xph.length = hio_read32l(f);
	xph.packing = hio_read8(f);
	xph.rows = version > 0x0102 ? hio_read16l(f) : hio_read8(f) + 1;

	/* Sanity check */
	if (xph.rows > 256)
	    goto err;

	xph.datasize = hio_read16l(f);
	hio_seek(f, xph.length - headsize, SEEK_CUR);

	r = xph.rows;
	if (r == 0)
	    r = 0x100;

	if (pattern_tracks_alloc(mod, i, r) < 0)
	    goto err;

	if (xph.datasize) {
	    int size = xph.datasize;

	    pat = patbuf = calloc(1, size);
	    if (patbuf == NULL)
		goto err;

	    hio_read(patbuf, 1, size, f);
	    for (j = 0; j < (mod->chn * r); j++) {

		/*if ((pat - patbuf) >= xph.datasize)
		    break;*/

		event = &EVENT(i, j % mod->chn, j / mod->chn);

		if (--size < 0)
		    goto err2;

		if ((b = *pat++) & XM_EVENT_PACKING) {
		    if (b & XM_EVENT_NOTE_FOLLOWS) {
			if (--size < 0)
			    goto err2;
			event->note = *pat++;
		    }
		    if (b & XM_EVENT_INSTRUMENT_FOLLOWS) {
			if (--size < 0)
			    goto err2;
			event->ins = *pat++;
		    }
		    if (b & XM_EVENT_VOLUME_FOLLOWS) {
			if (--size < 0)
			    goto err2;
			event->vol = *pat++;
		    }
		    if (b & XM_EVENT_FXTYPE_FOLLOWS) {
			if (--size < 0)
			    goto err2;
			event->fxt = *pat++;
		    }
		    if (b & XM_EVENT_FXPARM_FOLLOWS) {
			if (--size < 0)
			    goto err2;
			event->fxp = *pat++;
		    }
		} else {
                    size -=4;
		    if (size < 0)
			goto err2;
		    event->note = b;
		    event->ins = *pat++;
		    event->vol = *pat++;
		    event->fxt = *pat++;
		    event->fxp = *pat++;
		}

		/* Sanity check */
		switch (event->fxt) {
		case 18: case 19:
		case 22: case 23: case 24:
		case 26:
		case 28:
		case 30: case 31: case 32:
			event->fxt = 0;
		}
		if (event->fxt > 34) {
			event->fxt = 0;
		}

		if (event->note == 0x61) {
		    /* See OpenMPT keyoff+instr.xm test case */
		    if (event->fxt == 0x0e && MSN(event->fxp) == 0x0d) {
			event->note = XMP_KEY_OFF;
		    } else {
		    	event->note = event->ins ? XMP_KEY_FADE : XMP_KEY_OFF;
		    }
		} else if (event->note > 0) {
		    event->note += 12;
		}

		if (event->fxt == 0x0e) {
			switch (event->fxp) {
			case 0x43:
			case 0x73:
				event->fxp--;
				break;
			}
		}

		if (!event->vol)
		    continue;

		/* Volume set */
		if ((event->vol >= 0x10) && (event->vol <= 0x50)) {
		    event->vol -= 0x0f;
		    continue;
		}
		/* Volume column effects */
		switch (event->vol >> 4) {
		case 0x06:	/* Volume slide down */
		    event->f2t = FX_VOLSLIDE_2;
		    event->f2p = event->vol - 0x60;
		    break;
		case 0x07:	/* Volume slide up */
		    event->f2t = FX_VOLSLIDE_2;
		    event->f2p = (event->vol - 0x70) << 4;
		    break;
		case 0x08:	/* Fine volume slide down */
		    event->f2t = FX_EXTENDED;
		    event->f2p = (EX_F_VSLIDE_DN << 4) | (event->vol - 0x80);
		    break;
		case 0x09:	/* Fine volume slide up */
		    event->f2t = FX_EXTENDED;
		    event->f2p = (EX_F_VSLIDE_UP << 4) | (event->vol - 0x90);
		    break;
		case 0x0a:	/* Set vibrato speed */
		    event->f2t = FX_VIBRATO;
		    event->f2p = (event->vol - 0xa0) << 4;
		    break;
		case 0x0b:	/* Vibrato */
		    event->f2t = FX_VIBRATO;
		    event->f2p = event->vol - 0xb0;
		    break;
		case 0x0c:	/* Set panning */
		    event->f2t = FX_SETPAN;
		    event->f2p = (event->vol - 0xc0) << 4;
		    break;
		case 0x0d:	/* Pan slide left */
		    event->f2t = FX_PANSL_NOMEM;
		    event->f2p = (event->vol - 0xd0) << 4;
		    break;
		case 0x0e:	/* Pan slide right */
		    event->f2t = FX_PANSL_NOMEM;
		    event->f2p = event->vol - 0xe0;
		    break;
		case 0x0f:	/* Tone portamento */
		    event->f2t = FX_TONEPORTA;
		    event->f2p = (event->vol - 0xf0) << 4;

		    /* From OpenMPT TonePortamentoMemory.xm:
		     * "Another nice bug (...) is the combination of both
		     *  portamento commands (Mx and 3xx) in the same cell:
		     *  The 3xx parameter is ignored completely, and the Mx
		     *  parameter is doubled. (M2 3FF is the same as M4 000)
		     */
		    if (event->fxt == FX_TONEPORTA || event->fxt == FX_TONE_VSLIDE) {
			if (event->fxt == FX_TONEPORTA) {
			    event->fxt = 0;
			} else {
			    event->fxt = FX_VOLSLIDE;
			}
			event->fxp = 0;

			if (event->f2p < 0x80) {
				event->f2p <<= 1;
			} else {
				event->f2p = 0xff;
			}
		    }

		    /* From OpenMPT porta-offset.xm:
		     * "If there is a portamento command next to an offset
		     *  command, the offset command is ignored completely. In
		     *  particular, the offset parameter is not memorized."
		     */
		    if (event->fxt == FX_OFFSET && event->f2t == FX_TONEPORTA) {
			event->fxt = event->fxp = 0;
		    }
		    break;
		}
		event->vol = 0;
	    }
	    free(patbuf);
	}
    }

    /* Alloc one extra pattern */
    {
	int t = i * mod->chn;

	if (pattern_alloc(mod, i) < 0)
	    goto err;

	mod->xxp[i]->rows = 64;

	if (track_alloc(mod, t, 64) < 0)
	    goto err;

	for (j = 0; j < mod->chn; j++)
	    mod->xxp[i]->index[j] = t;
    }

    return 0;

  err2:
    free(patbuf);
  err:
    return -1;
}

/* Packed structures size */
#define XM_INST_HEADER_SIZE 33
#define XM_INST_SIZE 208

static int load_instruments(struct module_data *m, int version, HIO_HANDLE *f)
{
    struct xmp_module *mod = &m->mod;
    struct xm_instrument_header xih;
    struct xm_instrument xi;
    struct xm_sample_header xsh[16];
    int sample_num = 0;
    int i, j;

    D_(D_INFO "Instruments: %d", mod->ins);

    /* ESTIMATED value! We don't know the actual value at this point */
    mod->smp = MAX_SAMPLES;

    if (instrument_init(mod) < 0)
	return -1;

    for (i = 0; i < mod->ins; i++) {
	struct xmp_instrument *xxi = &mod->xxi[i];

	xih.size = hio_read32l(f);		/* Instrument size */

	/* Modules converted with MOD2XM 1.0 always say we have 31
	 * instruments, but file may end abruptly before that. This test
	 * will not work if file has trailing garbage.
	 */
	if (hio_eof(f)) {
		break;
	}

	hio_read(&xih.name, 22, 1, f);		/* Instrument name */
	xih.type = hio_read8(f);		/* Instrument type (always 0) */
	xih.samples = hio_read16l(f);		/* Number of samples */
	xih.sh_size = hio_read32l(f);		/* Sample header size */

	/* Sanity check */
	if (xih.samples > 0x10 || (xih.samples > 0 && xih.sh_size > 0x100)) {
		D_(D_CRIT "Sanity check: %d %d", xih.samples, xih.sh_size);
		return -1;
	}

	instrument_name(mod, i, xih.name, 22);

	xxi->nsm = xih.samples;
	if (xxi->nsm > 16)
	    xxi->nsm = 16;

	D_(D_INFO "[%2X] %-22.22s %2d", i, xxi->name, xxi->nsm);

	if (xxi->nsm) {
	    if (subinstrument_alloc(mod, i, xxi->nsm) < 0)
		return -1;
	    if (xih.size < XM_INST_HEADER_SIZE)
		return -1;

	    /* for BoobieSqueezer (see http://boobie.rotfl.at/)
	     * It works pretty much the same way as Impulse Tracker's sample
	     * only mode, where it will strip off the instrument data.
	     */
	    if (xih.size < XM_INST_HEADER_SIZE + XM_INST_SIZE) {
		memset(&xi, 0, sizeof (struct xm_instrument));
		hio_seek(f, xih.size - XM_INST_HEADER_SIZE, SEEK_CUR);
	    } else {
		hio_read(&xi.sample, 96, 1, f);	/* Sample map */
		for (j = 0; j < 24; j++)
		    xi.v_env[j] = hio_read16l(f); /* Points for volume envelope */
		for (j = 0; j < 24; j++)
		    xi.p_env[j] = hio_read16l(f); /* Points for pan envelope */
		xi.v_pts = hio_read8(f);	/* Number of volume points */
		xi.p_pts = hio_read8(f);	/* Number of pan points */
		xi.v_sus = hio_read8(f);	/* Volume sustain point */
		xi.v_start = hio_read8(f);	/* Volume loop start point */
		xi.v_end = hio_read8(f);	/* Volume loop end point */
		xi.p_sus = hio_read8(f);	/* Pan sustain point */
		xi.p_start = hio_read8(f);	/* Pan loop start point */
		xi.p_end = hio_read8(f);	/* Pan loop end point */
		xi.v_type = hio_read8(f);	/* Bit 0:On 1:Sustain 2:Loop */
		xi.p_type = hio_read8(f);	/* Bit 0:On 1:Sustain 2:Loop */
		xi.y_wave = hio_read8(f);	/* Vibrato waveform */
		xi.y_sweep = hio_read8(f);	/* Vibrato sweep */
		xi.y_depth = hio_read8(f);	/* Vibrato depth */
		xi.y_rate = hio_read8(f);	/* Vibrato rate */
		xi.v_fade = hio_read16l(f);	/* Volume fadeout */

		/* Skip reserved space */
		hio_seek(f, (int)xih.size - (XM_INST_HEADER_SIZE + XM_INST_SIZE), SEEK_CUR);

		/* Envelope */
		xxi->rls = xi.v_fade << 1;
		xxi->aei.npt = xi.v_pts;
		xxi->aei.sus = xi.v_sus;
		xxi->aei.lps = xi.v_start;
		xxi->aei.lpe = xi.v_end;
		xxi->aei.flg = xi.v_type;
		xxi->pei.npt = xi.p_pts;
		xxi->pei.sus = xi.p_sus;
		xxi->pei.lps = xi.p_start;
		xxi->pei.lpe = xi.p_end;
		xxi->pei.flg = xi.p_type;

		if (xxi->aei.npt <= 0 || xxi->aei.npt > 12 /*XMP_MAX_ENV_POINTS*/)
		    xxi->aei.flg &= ~XMP_ENVELOPE_ON;
		else
		    memcpy(xxi->aei.data, xi.v_env, xxi->aei.npt * 4);

		if (xxi->pei.npt <= 0 || xxi->pei.npt > 12 /*XMP_MAX_ENV_POINTS*/)
		    xxi->pei.flg &= ~XMP_ENVELOPE_ON;
		else
		    memcpy(xxi->pei.data, xi.p_env, xxi->pei.npt * 4);

		for (j = 12; j < 108; j++) {
		    xxi->map[j].ins = xi.sample[j - 12];
		    if (xxi->map[j].ins >= xxi->nsm)
			xxi->map[j].ins = -1;
		}
	    }

	    for (j = 0; j < xxi->nsm; j++, sample_num++) {
		struct xmp_subinstrument *sub = &xxi->sub[j];
		struct xmp_sample *xxs;

		if (sample_num >= mod->smp) {
		    mod->xxs = realloc_samples(mod->xxs, &mod->smp, mod->smp * 3 / 2);
		    if (mod->xxs == NULL)
			return -1;
		}
		xxs = &mod->xxs[sample_num];

		xsh[j].length = hio_read32l(f);		/* Sample length */

		/* Sanity check */
		if (xsh[j].length > MAX_SAMPLE_SIZE)
			return -1;

		xsh[j].loop_start = hio_read32l(f);	/* Sample loop start */
		xsh[j].loop_length = hio_read32l(f);	/* Sample loop length */
		xsh[j].volume = hio_read8(f);		/* Volume */
		xsh[j].finetune = hio_read8s(f);	/* Finetune (-128..+127) */
		xsh[j].type = hio_read8(f);		/* Flags */
		xsh[j].pan = hio_read8(f);		/* Panning (0-255) */
		xsh[j].relnote = hio_read8s(f);		/* Relative note number */
		xsh[j].reserved = hio_read8(f);
		hio_read(&xsh[j].name, 22, 1, f);	/* Sample_name */

		sub->vol = xsh[j].volume;
		sub->pan = xsh[j].pan;
		sub->xpo = xsh[j].relnote;
		sub->fin = xsh[j].finetune;
		sub->vwf = xi.y_wave;
		sub->vde = xi.y_depth;
		sub->vra = xi.y_rate;
		sub->vsw = xi.y_sweep;
		sub->sid = sample_num;

		copy_adjust(xxs->name, xsh[j].name, 22);

		xxs->len = xsh[j].length;
		xxs->lps = xsh[j].loop_start;
		xxs->lpe = xsh[j].loop_start + xsh[j].loop_length;

		xxs->flg = 0;
		if (xsh[j].type & XM_SAMPLE_16BIT) {
		    xxs->flg |= XMP_SAMPLE_16BIT;
		    xxs->len >>= 1;
		    xxs->lps >>= 1;
		    xxs->lpe >>= 1;
		}

		xxs->flg |= xsh[j].type & XM_LOOP_FORWARD ?
		    XMP_SAMPLE_LOOP : 0;
		xxs->flg |= xsh[j].type & XM_LOOP_PINGPONG ?
		    XMP_SAMPLE_LOOP | XMP_SAMPLE_LOOP_BIDIR : 0;
	    }
	    for (j = 0; j < xxi->nsm; j++) {
		struct xmp_subinstrument *sub = &xxi->sub[j];
		int flags;

		D_(D_INFO " %1x: %06x%c%06x %06x %c V%02x F%+04d P%02x R%+03d",
		    j, mod->xxs[sub->sid].len,
		    mod->xxs[sub->sid].flg & XMP_SAMPLE_16BIT ? '+' : ' ',
		    mod->xxs[sub->sid].lps,
		    mod->xxs[sub->sid].lpe,
		    mod->xxs[sub->sid].flg & XMP_SAMPLE_LOOP_BIDIR ? 'B' :
		    mod->xxs[sub->sid].flg & XMP_SAMPLE_LOOP ? 'L' : ' ',
		    sub->vol, sub->fin,
		    sub->pan, sub->xpo);

		flags = SAMPLE_FLAG_DIFF;
#ifndef LIBXMP_CORE_PLAYER
		if (xsh[j].reserved == 0xad) {
		    flags = SAMPLE_FLAG_ADPCM;
		}
#endif
		
		if (version > 0x0103) {
		    if (load_sample(m, f, flags,
					&mod->xxs[sub->sid], NULL) < 0) {
			return -1;
		    }
		}
	    }
	} else {
	    /* Sample size should be in struct xm_instrument according to
	     * the official format description, but FT2 actually puts it in
	     * struct xm_instrument header. There's a tracker or converter
	     * that follow the specs, so we must handle both cases (see 
	     * "Braintomb" by Jazztiz/ART).
	     */

	    /* Umm, Cyke O'Path <cyker@heatwave.co.uk> sent me a couple of
	     * mods ("Breath of the Wind" and "Broken Dimension") that
	     * reserve the instrument data space after the instrument header
	     * even if the number of instruments is set to 0. In these modules
	     * the instrument header size is marked as 263. The following
	     * generalization should take care of both cases.
	     */

	     hio_seek(f, (int)xih.size - XM_INST_HEADER_SIZE, SEEK_CUR);
	}
    }

    /* Final sample number adjustment */
    mod->xxs = realloc_samples(mod->xxs, &mod->smp, sample_num);
    if (mod->xxs == NULL)
	return -1;

    return 0;
}

static int xm_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
    struct xmp_module *mod = &m->mod;
    int i, j;
    struct xm_file_header xfh;
    char tracker_name[21];
    int len;

    LOAD_INIT();

    hio_read(&xfh.id, 17, 1, f);	/* ID text */
    hio_read(&xfh.name, 20, 1, f);	/* Module name */
    hio_read8(f);			/* 0x1a */
    hio_read(&xfh.tracker, 20, 1, f);	/* Tracker name */
    xfh.version = hio_read16l(f);	/* Version number, minor-major */
    xfh.headersz = hio_read32l(f);	/* Header size */
    xfh.songlen = hio_read16l(f);	/* Song length */
    xfh.restart = hio_read16l(f);	/* Restart position */
    xfh.channels = hio_read16l(f);	/* Number of channels */
    xfh.patterns = hio_read16l(f);	/* Number of patterns */
    xfh.instruments = hio_read16l(f);	/* Number of instruments */
    xfh.flags = hio_read16l(f);		/* 0=Amiga freq table, 1=Linear */
    xfh.tempo = hio_read16l(f);		/* Default tempo */
    xfh.bpm = hio_read16l(f);		/* Default BPM */

    /* Sanity checks */
    if (xfh.songlen > 256 || xfh.patterns > 256 || xfh.instruments > 255) {
	D_(D_CRIT "Sanity check: %d %d %d", xfh.songlen, xfh.patterns,
							xfh.instruments);
	return -1;
    }

    if (xfh.restart > 255 || xfh.channels > XMP_MAX_CHANNELS) {
	D_(D_CRIT "Sanity check: %d %d", xfh.restart, xfh.channels);
        return -1;
    }

    if (xfh.tempo >= 32 || xfh.bpm < 32 || xfh.bpm > 255) {
        if (memcmp("MED2XM", xfh.tracker, 6)) {
	    D_(D_CRIT "Sanity check: %d %d", xfh.tempo, xfh.bpm);
	    return -1;
        }
    }

    len = xfh.headersz - 0x14;
    if (len < 0 || len > 256) {
	D_(D_CRIT "Sanity check: %d", len);
	return -1;
    }

    /* Honor header size -- needed by BoobieSqueezer XMs */
    hio_read(&xfh.order, len, 1, f); /* Pattern order table */

    strncpy(mod->name, (char *)xfh.name, 20);

    mod->len = xfh.songlen;
    mod->chn = xfh.channels;
    mod->pat = xfh.patterns;
    mod->ins = xfh.instruments;
    mod->rst = xfh.restart;
    mod->spd = xfh.tempo;
    mod->bpm = xfh.bpm;
    mod->trk = mod->chn * mod->pat + 1;

    m->quirk |= xfh.flags & XM_LINEAR_PERIOD_MODE ? QUIRK_LINEAR : 0;

    memcpy(mod->xxo, xfh.order, mod->len);
    tracker_name[20] = 0;
    snprintf(tracker_name, 20, "%-20.20s", xfh.tracker);
    for (i = 20; i >= 0; i--) {
	if (tracker_name[i] == 0x20)
	    tracker_name[i] = 0;
	if (tracker_name[i])
	    break;
    }

    /* OpenMPT accurately emulates weird FT2 bugs */
    if (!strncmp(tracker_name, "FastTracker v2.00", 17) ||
        !strncmp(tracker_name, "OpenMPT ", 8)) {
	m->quirk |= QUIRK_FT2BUGS;
    }

#ifndef LIBXMP_CORE_PLAYER
    if (xfh.headersz == 0x0113) {
	strcpy(tracker_name, "unknown tracker");
        m->quirk &= ~QUIRK_FT2BUGS;
    } else if (*tracker_name == 0) {
	strcpy(tracker_name, "Digitrakker");	/* best guess */
        m->quirk &= ~QUIRK_FT2BUGS;
    }

    /* See MMD1 loader for explanation */
    if (!strncmp(tracker_name, "MED2XM by J.Pynnone", 19)) {
	if (mod->bpm <= 10)
	    mod->bpm = 125 * (0x35 - mod->bpm * 2) / 33;
        m->quirk &= ~QUIRK_FT2BUGS;
    }

    if (!strncmp(tracker_name, "FastTracker v 2.00", 18)) {
	strcpy(tracker_name, "old ModPlug Tracker");
        m->quirk &= ~QUIRK_FT2BUGS;
    }

    set_type(m, "%s XM %d.%02d", tracker_name,
				xfh.version >> 8, xfh.version & 0xff);
#else
    set_type(m, tracker_name);
#endif

    MODULE_INFO();

    /* Honor header size */

    hio_seek(f, start + xfh.headersz + 60, SEEK_SET);

    /* XM 1.02/1.03 has a different patterns and instruments order */

    if (xfh.version <= 0x0103) {
	if (load_instruments(m, xfh.version, f) < 0)
	    return -1;
	if (load_patterns(m, xfh.version, f) < 0)
	    return -1;
    } else {
	if (load_patterns(m, xfh.version, f) < 0)
	    return -1;
	if (load_instruments(m, xfh.version, f) < 0)
	    return -1;
    }

    D_(D_INFO "Stored samples: %d", mod->smp);

    /* XM 1.02 stores all samples after the patterns */

    if (xfh.version <= 0x0103) {
	for (i = 0; i < mod->ins; i++) {
	    for (j = 0; j < mod->xxi[i].nsm; j++) {
		int sid = mod->xxi[i].sub[j].sid;
		if (load_sample(m, f, SAMPLE_FLAG_DIFF,
				&mod->xxs[sid], NULL) < 0) {
		    return -1;
		}
	    }
	}
    }

    for (i = 0; i < mod->chn; i++) {
        mod->xxc[i].pan = 0x80;
    }

    m->quirk |= QUIRKS_FT2;
    m->read_event_type = READ_EVENT_FT2;

    return 0;
}

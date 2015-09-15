/* Extended Module Player
 * Copyright (C) 1996-2015 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

/* Loader for FC-M Packer modules based on the format description
 * written by Sylvain Chipaux (Asle/ReDoX). Modules sent by Sylvain
 * Chipaux.
 */

#include "loader.h"
#include "period.h"


struct fcm_instrument {
    uint16 size;
    uint8 finetune;
    uint8 volume;
    uint16 loop_start;
    uint16 loop_size;
};

struct fcm_header {
    uint8 magic[4];		/* 'FC-M' magic ID */
    uint8 vmaj;
    uint8 vmin;
    uint8 name_id[4];		/* 'NAME' pseudo chunk ID */
    uint8 name[20];
    uint8 inst_id[4];		/* 'INST' pseudo chunk ID */
    struct fcm_instrument ins[31];
    uint8 long_id[4];		/* 'LONG' pseudo chunk ID */
    uint8 len;
    uint8 rst;
    uint8 patt_id[4];		/* 'PATT' pseudo chunk ID */
};
    

int fcm_load(struct module_data *m, HIO_HANDLE *f)
{
    int i, j, k;
    struct xmp_event *event;
    struct fcm_header fh;
    uint8 fe[4];

    LOAD_INIT();

    hio_read (&fh, 1, sizeof (struct fcm_header), f);

    if (fh.magic[0] != 'F' || fh.magic[1] != 'C' || fh.magic[2] != '-' ||
	fh.magic[3] != 'M' || fh.name_id[0] != 'N')
	return -1;

    strncpy (mod->name, fh.name, 20);
    set_type(m, "FC-M %d.%d", fh.vmaj, fh.vmin);

    MODULE_INFO();

    mod->len = fh.len;

    hio_read (mod->xxo, 1, mod->len, f);

    for (mod->pat = i = 0; i < mod->len; i++) {
	if (mod->xxo[i] > mod->pat)
	    mod->pat = mod->xxo[i];
    }
    mod->pat++;

    mod->trk = mod->pat * mod->chn;

    INSTRUMENT_INIT();

    for (i = 0; i < mod->ins; i++) {
	B_ENDIAN16 (fh.ins[i].size);
	B_ENDIAN16 (fh.ins[i].loop_start);
	B_ENDIAN16 (fh.ins[i].loop_size);
	mod->xxi[i].sub = calloc(sizeof (struct xmp_subinstrument), 1);
	mod->xxs[i].len = 2 * fh.ins[i].size;
	mod->xxs[i].lps = 2 * fh.ins[i].loop_start;
	mod->xxs[i].lpe = mod->xxs[i].lps + 2 * fh.ins[i].loop_size;
	mod->xxs[i].flg = fh.ins[i].loop_size > 1 ? XMP_SAMPLE_LOOP : 0;
	mod->xxi[i].sub[0].fin = (int8)fh.ins[i].finetune << 4;
	mod->xxi[i].sub[0].vol = fh.ins[i].volume;
	mod->xxi[i].sub[0].pan = 0x80;
	mod->xxi[i].sub[0].sid = i;
	mod->xxi[i].nsm = !!(mod->xxs[i].len);
	mod->xxi[i].rls = 0xfff;
	if (mod->xxi[i].sub[0].fin > 48)
	    mod->xxi[i].sub[0].xpo = -1;
	if (mod->xxi[i].sub[0].fin < -48)
	    mod->xxi[i].sub[0].xpo = 1;

	if (V(1) && (strlen(mod->xxi[i].name) || mod->xxs[i].len > 2)) {
	    report ("[%2X] %04x %04x %04x %c V%02x %+d\n",
		i, mod->xxs[i].len, mod->xxs[i].lps, mod->xxs[i].lpe,
		fh.ins[i].loop_size > 1 ? 'L' : ' ',
		mod->xxi[i].sub[0].vol, mod->xxi[i].sub[0].fin >> 4);
	}
    }

    PATTERN_INIT();

    /* Load and convert patterns */
    if (V(0))
	report ("Stored patterns: %d ", mod->pat);

    hio_read (fe, 4, 1, f);	/* Skip 'SONG' pseudo chunk ID */

    for (i = 0; i < mod->pat; i++) {
	PATTERN_ALLOC (i);
	mod->xxp[i]->rows = 64;
	TRACK_ALLOC (i);
	for (j = 0; j < 64; j++) {
	    for (k = 0; k < 4; k++) {
		event = &EVENT (i, k, j);
		hio_read (fe, 4, 1, f);
		cvt_pt_event (event, fe);
	    }
	}

	if (V(0))
	    report (".");
    }

    mod->flg |= XXM_FLG_MODRNG;

    /* Load samples */

    hio_read (fe, 4, 1, f);	/* Skip 'SAMP' pseudo chunk ID */

    if (V(0))
	report ("\nStored samples : %d ", mod->smp);
    for (i = 0; i < mod->smp; i++) {
	if (!mod->xxs[i].len)
	    continue;
	load_sample(m, f, 0, &mod->xxs[mod->xxi[i].sub[0].sid], NULL);
	if (V(0))
	    report (".");
    }
    if (V(0))
	report ("\n");

    return 0;
}

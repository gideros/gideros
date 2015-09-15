/* Extended Module Player
 * Copyright (C) 1996-2015 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

/*
 * From http://amp.dascene.net/faq.php:
 * [Face The Music is an] Amiga tracker created by Jörg W. Schmidt in 1990
 * for Maxon Computer GmbH. Face the Music delivers: 8 channels, line-based
 * editor with S.E.L. (Sound Effect Language).
 */
#include "loader.h"

struct ftm_instrument {
	uint8 name[30];		/* Instrument name */
	uint8 unknown[2];
};

struct ftm_header {
	uint8 id[4];		/* "FTMN" ID string */
	uint8 ver;		/* Version ?!? (0x03) */
	uint8 nos;		/* Number of samples (?) */
	uint8 unknown[10];
	uint8 title[32];	/* Module title */
	uint8 author[32];	/* Module author */
	uint8 unknown2[2];
};

int ftm_load(HIO_HANDLE * f)
{
	int i, j, k;
	struct xmp_event *event;
	struct ftm_header fh;
	struct ftm_instrument si;
	uint8 b1, b2, b3;

	LOAD_INIT();

	hio_read(&fh.id, 4, 1, f);
	if (memcmp(fh.id, "FTMN", 4))
		return -1;

	fh.ver = hio_read8(f);
	fh.nos = hio_read8(f);
	hio_read16b(f);
	hio_read32b(f);
	hio_read32b(f);
	hio_read(&fh.title, 32, 1, f);
	hio_read(&fh.author, 32, 1, f);
	hio_read16b(f);

	//mod->len = fh.len;
	//mod->pat = fh.pat;
	mod->ins = fh.nos;
	mod->smp = mod->ins;
	mod->trk = mod->pat * mod->chn;
	for (i = 0; i < mod->len; i++)
		mod->xxo[i] = fh.order[i];

	set_type(m, "Face The Music");
	MODULE_INFO();
	PATTERN_INIT();

	/* Load and convert patterns */
	if (V(0))
		report("Stored patterns: %d ", mod->pat);
	for (i = 0; i < mod->pat; i++) {
		PATTERN_ALLOC(i);
		mod->xxp[i]->rows = 64;
		TRACK_ALLOC(i);
		for (j = 0; j < 4; j++) {
		}

		reportv(ctx, 0, ".");
	}

	INSTRUMENT_INIT();
	reportv(ctx, 0, "\nStored samples : %d ", mod->smp);

	for (i = 0; i < mod->smp; i++) {
		reportv(ctx, 0, ".");
	}

	reportv(ctx, 0, "\n");
	mod->flg |= XXM_FLG_MODRNG;

	return 0;
}

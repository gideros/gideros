/* Extended Module Player
 * Copyright (C) 1996-2015 Claudio Matsuoka and Hipolito Carraro Jr
 *
 * This file is part of the Extended Module Player and is distributed
 * under the terms of the GNU Lesser General Public License. See COPYING.LIB
 * for more information.
 */

/* Format specs from MTPng 4.3.1 by Ian Schmitd and deMODifier by Bret Victor
 * http://home.cfl.rr.com/ischmidt/warez.html
 * http://worrydream.com/media/demodifier.tgz
 */

/* From the deMODifier readme:
 *
 * SoundSmith was arguably the most popular music authoring tool for the
 * Apple IIgs.  Introduced in the IIgs's heyday (which was, accurately 
 * enough, just about one day), this software inspired the creation
 * of countless numbers of IIgs-specific tunes, several of which were 
 * actually worth listening to.  
 */

#include <string.h>
#include "loader.h"
#include "asif.h"


static int mtp_test (HIO_HANDLE *, char *, const int);
static int mtp_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader mtp_loader = {
	"Soundsmith/MegaTracker (MTP)",
	mtp_test,
	mtp_load
};

static int mtp_test(HIO_HANDLE *f, char *t, const int start)
{
	char buf[6];

	if (hio_read(buf, 1, 6, f) < 6)
		return -1;

	if (memcmp(buf, "SONGOK", 6) && memcmp(buf, "IAN92a", 6))
		return -1;

	read_title(f, t, 0);

	return 0;
}




#define NAME_SIZE 255


static int mtp_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
	struct xmp_module *mod = &m->mod;
	struct xmp_event *event;
	int i, j, k;
	uint8 buffer[25];
	int blocksize;

	LOAD_INIT();

	hio_read(buffer, 6, 1, f);

	if (!memcmp(buffer, "SONGOK", 6))
		set_type(m, "IIgs SoundSmith");
	else if (!memcmp(buffer, "IAN92a", 8))
		set_type(m, "IIgs MegaTracker");
	else
		return -1;

	blocksize = hio_read16l(f);
	mod->spd = hio_read16l(f);
	hio_seek(f, 10, SEEK_CUR);		/* skip 10 reserved bytes */
	
	mod->ins = mod->smp = 15;
	if (instrument_init(mod) < 0)
		return -1;

	for (i = 0; i < mod->ins; i++) {
		if (subinstrument_alloc(mod, i, 1) < 0)
			return -1;

		hio_read(buffer, 1, 22, f);
		if (buffer[0]) {
			buffer[buffer[0] + 1] = 0;
			instrument_name(mod, i, buffer + 1, 22);
		}
		hio_read16l(f);		/* skip 2 reserved bytes */
		mod->xxi[i].sub[0].vol = hio_read8(f) >> 2;
		mod->xxi[i].sub[0].pan = 0x80;
		hio_seek(f, 5, SEEK_CUR);	/* skip 5 bytes */
	}

	mod->len = hio_read8(f) & 0x7f;
	hio_read8(f);
	hio_read(mod->xxo, 1, 128, f);

	MODULE_INFO();

	hio_seek(f, start + 600, SEEK_SET);

	mod->chn = 14;
	mod->pat = blocksize / (14 * 64);
	mod->trk = mod->pat * mod->chn;

	if (pattern_init(mod) < 0)
		return -1;

	/* Read and convert patterns */
	D_(D_INFO "Stored patterns: %d", mod->pat);

	/* Load notes */
	for (i = 0; i < mod->pat; i++) {
		if (pattern_tracks_alloc(mod, i, 64) < 0)
			return -1;

		for (j = 0; j < mod->xxp[i]->rows; j++) {
			for (k = 0; k < mod->chn; k++) {
				event = &EVENT(i, k, j);
				event->note = hio_read8(f);;
				if (event->note)
					event->note += 24;
			}
		}
	}

	/* Load fx1 */
	for (i = 0; i < mod->pat; i++) {
		for (j = 0; j < mod->xxp[i]->rows; j++) {
			for (k = 0; k < mod->chn; k++) {
				uint8 x;
				event = &EVENT(i, k, j);
				x = hio_read8(f);;
				event->ins = x >> 4;

				switch (x & 0x0f) {
				case 0x00:
					event->fxt = FX_ARPEGGIO;
					break;
				case 0x03:
					event->fxt = FX_VOLSET;
					break;
				case 0x05:
					event->fxt = FX_VOLSLIDE_DN;
					break;
				case 0x06:
					event->fxt = FX_VOLSLIDE_UP;
					break;
				case 0x0f:
					event->fxt = FX_SPEED;
					break;
				}
			}
		}
	}

	/* Load fx2 */
	for (i = 0; i < mod->pat; i++) {
		for (j = 0; j < mod->xxp[i]->rows; j++) {
			for (k = 0; k < mod->chn; k++) {
				event = &EVENT(i, k, j);
				event->fxp = hio_read8(f);;

				switch (event->fxt) {
				case FX_VOLSET:
				case FX_VOLSLIDE_DN:
				case FX_VOLSLIDE_UP:
					event->fxp >>= 2;
				}
			}
		}
	}

	/* Read instrument data */
	D_(D_INFO "Instruments    : %d ", mod->ins);

	for (i = 0; i < mod->ins; i++) {
		HIO_HANDLE *s;
		char filename[1024];

		if (!mod->xxi[i].name[0])
			continue;

		strncpy(filename, m->dirname, NAME_SIZE);
		if (*filename)
			strncat(filename, "/", NAME_SIZE);
		strncat(filename, (char *)mod->xxi[i].name, NAME_SIZE);

		if ((s = hio_open_file(filename, "rb")) != NULL) {
			asif_load(m, s, i);
			hio_close(s);
		}

#if 0
		mod->xxs[i].lps = 0;
		mod->xxs[i].lpe = 0;
		mod->xxs[i].flg = mod->xxs[i].lpe > 0 ? XMP_SAMPLE_LOOP : 0;
		mod->xxi[i].sub[0].fin = 0;
		mod->xxi[i].sub[0].pan = 0x80;
#endif

		D_(D_INFO "[%2X] %-22.22s %04x %04x %04x %c V%02x", i,
				mod->xxi[i].name,
				mod->xxs[i].len, mod->xxs[i].lps, mod->xxs[i].lpe,
				mod->xxs[i].flg & XMP_SAMPLE_LOOP ? 'L' : ' ',
				mod->xxi[i].sub[0].vol);
	}

	return 0;
}

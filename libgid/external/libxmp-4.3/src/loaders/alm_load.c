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

/* ALM (Aley's Module) is a module format used on 8bit computers. It was
 * designed to be usable on Sam Coupe (CPU Z80 6MHz) and PC XT. The ALM file
 * format is very simple and it have no special effects, so every computer
 * can play the ALMs.
 *
 * Note: xmp's module loading mechanism was not designed to load samples
 * from different files. Using *module into a global variable is a hack.
 */

#include "loader.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


static int alm_test (HIO_HANDLE *, char *, const int);
static int alm_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader alm_loader = {
    "Aley Keptr (ALM)",
    alm_test,
    alm_load
};

static int alm_test(HIO_HANDLE *f, char *t, const int start)
{
    char buf[7];

    if (HIO_HANDLE_TYPE(f) != HIO_HANDLE_TYPE_FILE)
	return -1;

    if (hio_read(buf, 1, 7, f) < 7)
	return -1;

    if (memcmp(buf, "ALEYMOD", 7) && memcmp(buf, "ALEY MO", 7))
	return -1;

    read_title(f, t, 0);

    return 0;
}



struct alm_file_header {
    uint8 id[7];		/* "ALEY MO" or "ALEYMOD" */
    uint8 speed;		/* Only in versions 1.1 and 1.2 */
    uint8 length;		/* Length of module */
    uint8 restart;		/* Restart position */
    uint8 order[128];		/* Pattern sequence */
};

#define NAME_SIZE 255

static int alm_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
    struct xmp_module *mod = &m->mod;
    int i, j;
    struct alm_file_header afh;
    struct xmp_event *event;
    struct stat stat;
    uint8 b;
    char *basename;
    char filename[NAME_SIZE];
    char modulename[NAME_SIZE];

    LOAD_INIT();

    hio_read(&afh.id, 7, 1, f);

    if (!strncmp((char *)afh.id, "ALEYMOD", 7))		/* Version 1.0 */
	mod->spd = afh.speed / 2;

    strncpy(modulename, m->filename, NAME_SIZE);
    basename = strtok (modulename, ".");

    afh.speed = hio_read8(f);
    afh.length = hio_read8(f);
    afh.restart = hio_read8(f);
    hio_read(&afh.order, 128, 1, f);

    mod->len = afh.length;
    mod->rst = afh.restart;
    memcpy (mod->xxo, afh.order, mod->len);

    for (mod->pat = i = 0; i < mod->len; i++)
	if (mod->pat < afh.order[i])
	    mod->pat = afh.order[i];
    mod->pat++;

    mod->ins = 31;
    mod->trk = mod->pat * mod->chn;
    mod->smp = mod->ins;
    m->c4rate = C4_NTSC_RATE;

    set_type(m, "Aley's Module");

    MODULE_INFO();

    if (pattern_init(mod) < 0)
	return -1;

    /* Read and convert patterns */
    D_(D_INFO "Stored patterns: %d", mod->pat);

    for (i = 0; i < mod->pat; i++) {
	if (pattern_tracks_alloc(mod, i, 64) < 0)
		return -1;

	for (j = 0; j < 64 * mod->chn; j++) {
	    event = &EVENT (i, j % mod->chn, j / mod->chn);
	    b = hio_read8(f);
	    if (b)
		event->note = (b == 37) ? 0x61 : b + 48;
	    event->ins = hio_read8(f);
	}
    }

    if (instrument_init(mod) < 0)
	return -1;

    /* Read and convert instruments and samples */

    D_(D_INFO "Loading samples: %d", mod->ins);

    for (i = 0; i < mod->ins; i++) {
	HIO_HANDLE *s;

	if (subinstrument_alloc(mod, i, 1) < 0)
	    return -1;

	mod->xxi[i].sub = calloc(sizeof (struct xmp_subinstrument), 1);
	snprintf(filename, NAME_SIZE, "%s.%d", basename, i + 1);
	s = hio_open_file(filename, "rb");

	if (s == NULL)
	    continue;

	mod->xxi[i].nsm = 1;

	hio_stat(s, &stat);
	b = hio_read8(s);		/* Get first octet */
	mod->xxs[i].len = stat.st_size - 5 * !b;

	if (!b) {		/* Instrument with header */
	    mod->xxs[i].lps = hio_read16l(f);
	    mod->xxs[i].lpe = hio_read16l(f);
	    mod->xxs[i].flg = mod->xxs[i].lpe > mod->xxs[i].lps ? XMP_SAMPLE_LOOP : 0;
	} else {
	    hio_seek(s, 0, SEEK_SET);
	}

	mod->xxi[i].sub[0].pan = 0x80;
	mod->xxi[i].sub[0].vol = 0x40;
	mod->xxi[i].sub[0].sid = i;

	D_(D_INFO "[%2X] %-14.14s %04x %04x %04x %c V%02x", i,
		filename, mod->xxs[i].len, mod->xxs[i].lps, mod->xxs[i].lpe,
		mod->xxs[i].flg & XMP_SAMPLE_LOOP ? 'L' : ' ', mod->xxi[i].sub[0].vol);

	if (load_sample(m, s, SAMPLE_FLAG_UNS, &mod->xxs[i], NULL) < 0)
	    return -1;

	hio_close(s);
    }

    /* ALM is LRLR, not LRRL */
    for (i = 0; i < mod->chn; i++)
	mod->xxc[i].pan = DEFPAN((i % 2) * 0xff);

    return 0;
}

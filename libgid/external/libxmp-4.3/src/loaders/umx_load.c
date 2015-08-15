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

#include "loader.h"

#define TEST_SIZE 1500

#define MAGIC_UMX	MAGIC4(0xc1,0x83,0x2a,0x9e)
#define MAGIC_IMPM	MAGIC4('I','M','P','M')
#define MAGIC_SCRM	MAGIC4('S','C','R','M')
#define MAGIC_M_K_	MAGIC4('M','.','K','.')

extern const struct format_loader xm_loader;
extern const struct format_loader it_loader;
extern const struct format_loader s3m_loader;
extern const struct format_loader mod_loader;

static int umx_test (HIO_HANDLE *, char *, const int);
static int umx_load (struct module_data *, HIO_HANDLE *, const int);

const struct format_loader umx_loader = {
	"Epic Games UMX",
	umx_test,
	umx_load
};

static int umx_test(HIO_HANDLE *f, char *t, const int start)
{
	int i, offset = -1;
	uint8 buf[TEST_SIZE], *b = buf;
	uint32 id;

	if (hio_read(buf, 1, TEST_SIZE, f) < TEST_SIZE)
		return -1;
;
	id = readmem32b(b);

	if (id != MAGIC_UMX)
		return -1;

	for (i = 0; i < TEST_SIZE; i++, b++) {
		id = readmem32b(b);

		if (!memcmp(b, "Extended Module:", 16)) {
			offset = i;
			break;
		}
		if (id == MAGIC_IMPM) { 
			offset = i;
			break;
		}
		if (i > 44 && id == MAGIC_SCRM) { 
			offset = i - 44;
			break;
		}
		if (i > 1080 && id == MAGIC_M_K_) { 
			offset = i - 1080;
			break;
		}
	}
	
	if (offset < 0)
		return -1;

	return 0;
}

static int umx_load(struct module_data *m, HIO_HANDLE *f, const int start)
{
	int i;
	uint8 buf[TEST_SIZE], *b = buf;
	uint32 id;

	LOAD_INIT();

	D_(D_INFO "Container type : Epic Games UMX");

	hio_read(buf, 1, TEST_SIZE, f);

	for (i = 0; i < TEST_SIZE; i++, b++) {
		id = readmem32b(b);

		if (!memcmp(b, "Extended Module:", 16))
			return xm_loader.loader(m, f, i);

		if (id == MAGIC_IMPM)
			return it_loader.loader(m, f, i);

		if (i > 44 && id == MAGIC_SCRM)
			return s3m_loader.loader(m, f, i - 44);

		if (i > 1080 && id == MAGIC_M_K_)
			return mod_loader.loader(m, f, i - 1080);
	}
	
	return -1;
}






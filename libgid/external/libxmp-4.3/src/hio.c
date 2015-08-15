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

#include "stdio2.h"
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include "common.h"
#include "hio.h"
#include "mdataio.h"

static long get_size(FILE *f)
{
	long size, pos;

	pos = ftell(f);
	if (pos >= 0) {
		fseek(f, 0, SEEK_END);
		size = ftell(f);
		fseek(f, pos, SEEK_SET);
		return size;
	} else {
		return pos;
	}
}

int8 hio_read8s(HIO_HANDLE *h)
{
	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		return read8s(h->handle.file);
	case HIO_HANDLE_TYPE_MEMORY:
		return mread8s(h->handle.mem);
	default:
		return 0;
	}
}

uint8 hio_read8(HIO_HANDLE *h)
{
	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		return read8(h->handle.file);
	case HIO_HANDLE_TYPE_MEMORY:
		return mread8(h->handle.mem);
	default:
		return 0;
	}
}

uint16 hio_read16l(HIO_HANDLE *h)
{
	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		return read16l(h->handle.file);
	case HIO_HANDLE_TYPE_MEMORY:
		return mread16l(h->handle.mem);
	default:
		return 0;
	}
}

uint16 hio_read16b(HIO_HANDLE *h)
{
	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		return read16b(h->handle.file);
	case HIO_HANDLE_TYPE_MEMORY:
		return mread16b(h->handle.mem);
	default:
		return 0;
	}
}

uint32 hio_read24l(HIO_HANDLE *h)
{
	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		return read24l(h->handle.file); 
	case HIO_HANDLE_TYPE_MEMORY:
		return mread24l(h->handle.mem); 
	default:
		return 0;
	}
}

uint32 hio_read24b(HIO_HANDLE *h)
{
	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		return read24b(h->handle.file);
	case HIO_HANDLE_TYPE_MEMORY:
		return mread24b(h->handle.mem);
	default:
		return 0;
	}
}

uint32 hio_read32l(HIO_HANDLE *h)
{
	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		return read32l(h->handle.file);
	case HIO_HANDLE_TYPE_MEMORY:
		return mread32l(h->handle.mem);
	default:
		return 0;
	}
}

uint32 hio_read32b(HIO_HANDLE *h)
{
	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		return read32b(h->handle.file);
	case HIO_HANDLE_TYPE_MEMORY:
		return mread32b(h->handle.mem);
	default:
		return 0;
	}
}

size_t hio_read(void *buf, size_t size, size_t num, HIO_HANDLE *h)
{
	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		return fread(buf, size, num, h->handle.file);
	case HIO_HANDLE_TYPE_MEMORY:
		return mread(buf, size, num, h->handle.mem);
	default:
		return 0;
	}
}

int hio_seek(HIO_HANDLE *h, long offset, int whence)
{
	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		return fseek(h->handle.file, offset, whence);
	case HIO_HANDLE_TYPE_MEMORY:
		return mseek(h->handle.mem, offset, whence);
	default:
		return -1;
	}
}

long hio_tell(HIO_HANDLE *h)
{
	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		return ftell(h->handle.file);
	case HIO_HANDLE_TYPE_MEMORY:
		return mtell(h->handle.mem);
	default:
		return -1;
	}
}

int hio_eof(HIO_HANDLE *h)
{
	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		return feof(h->handle.file);
	case HIO_HANDLE_TYPE_MEMORY:
		return meof(h->handle.mem);
	default:
		return EOF;
	}
}

HIO_HANDLE *hio_open(void *path, char *mode)
{
	HIO_HANDLE *h;

	h = (HIO_HANDLE *)malloc(sizeof (HIO_HANDLE));
	if (h == NULL)
		goto err;
	
	h->type = HIO_HANDLE_TYPE_FILE;
	h->handle.file = fopen(path, mode);
	if (h->handle.file == NULL)
		goto err2;

	h->size = get_size(h->handle.file);

	return h;

    err2:
	free(h);
    err:
	return NULL;
}

HIO_HANDLE *hio_open_mem(void *ptr, long size)
{
	HIO_HANDLE *h;

	h = (HIO_HANDLE *)malloc(sizeof (HIO_HANDLE));
	if (h == NULL)
		return NULL;
	
	h->type = HIO_HANDLE_TYPE_MEMORY;
	h->handle.mem = mopen(ptr, size);
	h->size = size;

	return h;
}

HIO_HANDLE *hio_open_file(FILE *f)
{
	HIO_HANDLE *h;

	h = (HIO_HANDLE *)malloc(sizeof (HIO_HANDLE));
	if (h == NULL)
		return NULL;
	
	h->type = HIO_HANDLE_TYPE_FILE;
	h->handle.file = f /*fdopen(fileno(f), "rb")*/;
	h->size = get_size(f);

	return h;
}

int hio_close(HIO_HANDLE *h)
{
	int ret;

	switch (HIO_HANDLE_TYPE(h)) {
	case HIO_HANDLE_TYPE_FILE:
		ret = fclose(h->handle.file);
		break;
	case HIO_HANDLE_TYPE_MEMORY:
		ret = mclose(h->handle.mem);
		break;
	default:
		ret = -1;
	}

	free(h);
	return ret;
}

long hio_size(HIO_HANDLE *h)
{
	return h->size;
}

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
#include <string.h>
#include "common.h"
#include "list.h"
#include "iff.h"

#include "loader.h"

struct iff_data {
	struct list_head iff_list;
	int id_size;
	int flags;
};

iff_handle iff_new()
{
	struct iff_data *data;

	data = malloc(sizeof(struct iff_data));
	if (data == NULL)
		return NULL;

	INIT_LIST_HEAD(&data->iff_list);
	data->id_size = 4;
	data->flags = 0;

	return (iff_handle) data;
}

static int iff_chunk(iff_handle opaque, struct module_data *m, HIO_HANDLE *f, void *parm)
{
	struct iff_data *data = (struct iff_data *)opaque;
	int size;
	char id[17] = "";

	if (hio_read(id, 1, data->id_size, f) != data->id_size)
		return 1;

	if (data->flags & IFF_SKIP_EMBEDDED) {
		/* embedded RIFF hack */
		if (!strncmp(id, "RIFF", 4)) {
			hio_read32b(f);
			hio_read32b(f);
			/* read first chunk ID instead */
			hio_read(id, 1, data->id_size, f);
		}
	}

	size = (data->flags & IFF_LITTLE_ENDIAN) ? hio_read32l(f) : hio_read32b(f);

	if (data->flags & IFF_CHUNK_ALIGN2)
		size = (size + 1) & ~1;

	if (data->flags & IFF_CHUNK_ALIGN4)
		size = (size + 3) & ~3;

	if (data->flags & IFF_FULL_CHUNK_SIZE)
		size -= data->id_size + 4;

	if (size < 0)
		return -1;

	return iff_process(opaque, m, id, size, f, parm);
}

int iff_load(iff_handle opaque, struct module_data *m, HIO_HANDLE *f, void *parm)
{
	int ret;

	while (!hio_eof(f)) {
		ret = iff_chunk(opaque, m, f, parm);

		if (ret > 0)
			break;

		if (ret < 0)
			return -1;
	}

	return 0;
}

int iff_register(iff_handle opaque, char *id,
	int (*loader)(struct module_data *, int, HIO_HANDLE *, void *))
{
	struct iff_data *data = (struct iff_data *)opaque;
	struct iff_info *f;

	f = malloc(sizeof(struct iff_info));
	if (f == NULL)
		return -1;

	strncpy(f->id, id, 5);
	f->loader = loader;

	list_add_tail(&f->list, &data->iff_list);

	return 0;
}

void iff_release(iff_handle opaque)
{
	struct iff_data *data = (struct iff_data *)opaque;
	struct list_head *tmp;
	struct iff_info *i;

	/* can't use list_for_each, we free the node before incrementing */
	for (tmp = (&data->iff_list)->next; tmp != (&data->iff_list);) {
		i = list_entry(tmp, struct iff_info, list);
		list_del(&i->list);
		tmp = tmp->next;
		free(i);
	}

	free(data);
}

int iff_process(iff_handle opaque, struct module_data *m, char *id, long size,
		HIO_HANDLE *f, void *parm)
{
	struct iff_data *data = (struct iff_data *)opaque;
	struct list_head *tmp;
	struct iff_info *i;
	int pos;

	pos = hio_tell(f);

	list_for_each(tmp, &data->iff_list) {
		i = list_entry(tmp, struct iff_info, list);
		if (id && !strncmp(id, i->id, data->id_size)) {
			D_(D_WARN "Load IFF chunk %s (%ld) @ %d",
							id, size, pos);
			if (i->loader(m, size, f, parm) < 0)
				return -1;
			break;
		}
	}

	hio_seek(f, pos + size, SEEK_SET);

	return 0;
}

/* Functions to tune IFF mutations */

void iff_id_size(iff_handle opaque, int n)
{
	struct iff_data *data = (struct iff_data *)opaque;

	data->id_size = n;
}

void iff_set_quirk(iff_handle opaque, int i)
{
	struct iff_data *data = (struct iff_data *)opaque;

	data->flags |= i;
}

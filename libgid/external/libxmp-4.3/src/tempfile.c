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

#ifdef __SUNPRO_C
#pragma error_messages (off,E_EMPTY_TRANSLATION_UNIT)
#endif

#ifndef LIBXMP_CORE_PLAYER

#include "stdio2.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include "tempfile.h"

#ifdef WIN32

int mkstemp(char *);

static int get_temp_dir(char *buf, int size)
{
	const char def[] = "C:\\WINDOWS\\TEMP";
	char *tmp = getenv("TEMP");

	strncpy(buf, tmp ? tmp : def, size);
	strncat(buf, "\\", size);

	return 0;
}

#elif defined __AMIGA__

static int get_temp_dir(char *buf, int size)
{
	strncpy(buf, "T:", size);
	return 0;
}

#elif defined ANDROID

#include <sys/types.h>
#include <sys/stat.h>

static int get_temp_dir(char *buf, int size)
{
#define APPDIR "/sdcard/Xmp for Android"
	struct stat st;
	if (stat(APPDIR, &st) < 0) {
		if (mkdir(APPDIR, 0777) < 0)
			return -1;
	}
	if (stat(APPDIR "/tmp", &st) < 0) {
		if (mkdir(APPDIR "/tmp", 0777) < 0)
			return -1;
	}
	strncpy(buf, APPDIR "/tmp/", size);

	return 0;
}

#else

static int get_temp_dir(char *buf, int size)
{
	char *tmp = getenv("TMPDIR");

	if (tmp) {
		snprintf(buf, size, "%s/", tmp);
	} else {
		strncpy(buf, "/tmp/", size);
	}

	return 0;
}

#endif


FILE *make_temp_file(char **filename) {
	char tmp[PATH_MAX];
	FILE *temp;
	int fd;

	if (get_temp_dir(tmp, PATH_MAX) < 0)
		return NULL;

	strncat(tmp, "xmp_XXXXXX", PATH_MAX - 10);

	if ((*filename = strdup(tmp)) == NULL)
		goto err;

	if ((fd = mkstemp(*filename)) < 0)
		goto err2;

	if ((temp = fdopen(fd, "w+b")) == NULL)
		goto err3;

	return temp;

    err3:
	close(fd);
    err2:
	free(*filename);
    err:
	return NULL;
}

/*
 * Windows doesn't allow you to unlink an open file, so we changed the
 * temp file cleanup system to remove temporary files after we close it
 */
void unlink_temp_file(char *temp)
{
	if (temp) {
		unlink(temp);
		free(temp);
	}
}

#endif

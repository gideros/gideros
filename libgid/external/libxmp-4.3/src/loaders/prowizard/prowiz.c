/*
 * Pro-Wizard_1.c
 *
 * Copyright (C) 1997-1999 Sylvain "Asle" Chipaux
 * Copyright (C) 2006-2007 Claudio Matsuoka
 */

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "xmp.h"

#include "prowiz.h"


const struct pw_format *const pw_format[NUM_PW_FORMATS + 1] = {
	/* With signature */
	&pw_ac1d,
	&pw_fchs,
	&pw_fcm,
	&pw_fuzz,
	&pw_hrt,
	/* &pw_kris, */
	&pw_ksm,
	&pw_mp_id,
	&pw_ntp,
	&pw_p18a,
	&pw_p10c,
	&pw_pru1,
	&pw_pru2,
	&pw_pha,
	&pw_wn,
	&pw_unic_id,
	&pw_tp3,
	&pw_skyt,

	/* No signature */
	&pw_xann,
	&pw_mp_noid,	/* Must check before Heatseeker */
	&pw_di,
	&pw_eu,
	&pw_p4x,
	&pw_pp21,
	&pw_pp30,
	&pw_p50a,
	&pw_p60a,
	&pw_p61a,
	&pw_nru,
	&pw_np2,
	&pw_np1,
	&pw_np3,
	&pw_zen,
	&pw_unic_emptyid,
	&pw_unic_noid,
	&pw_unic2,
	&pw_crb,
	&pw_tdd,
	&pw_starpack,
	&pw_gmc,
	&pw_titanics,
	NULL
};

int pw_move_data(FILE *out, HIO_HANDLE *in, int len)
{
	uint8 buf[1024];
	int l;

	do {
		l = hio_read(buf, 1, len > 1024 ? 1024 : len, in);
		fwrite(buf, 1, l, out);
		len -= l;
	} while (l > 0 && len > 0);

	return 0;
}

int pw_write_zero(FILE *out, int len)
{
	uint8 buf[1024];
	int l;
	
	do {
		l = len > 1024 ? 1024 : len;
		memset(buf, 0, l);
		fwrite(buf, 1, l, out);
		len -= l;
	} while (l > 0 && len > 0);

	return 0;
}

int pw_wizardry(HIO_HANDLE *file_in, FILE *file_out, char **name)
{
	int in_size;
	uint8 *data;
	char title[21];
	int i;

	in_size = hio_size(file_in);

	/* printf ("input file size : %d\n", in_size); */
	if (in_size < MIN_FILE_LENGHT)
		return -2;

	/* alloc mem */
	data = (uint8 *)malloc (in_size + 4096);	/* slack added */
	if (data == NULL) {
		/*perror("Couldn't allocate memory");*/
		return -1;
	}
	hio_read(data, in_size, 1, file_in);


  /********************************************************************/
  /**************************   SEARCH   ******************************/
  /********************************************************************/

	for (i = 0; pw_format[i] != NULL; i++) {
		D_("checking format: %s", pw_format[i]->name);
		if (pw_format[i]->test(data, title, in_size) >= 0)
			break;
	}

	if (pw_format[i] == NULL) {
		free(data);
		return -1;
	}

	hio_seek(file_in, 0, SEEK_SET);
	if (pw_format[i]->depack(file_in, file_out) < 0) {
		free(data);
		return -1;
	}

	fflush(file_out);
	free(data);

	if (name != NULL) {
		*name = pw_format[i]->name;
	}

	return 0;
}

int pw_check(unsigned char *b, int s, struct xmp_test_info *info)
{
	int i, res;
	char title[21];

	for (i = 0; pw_format[i] != NULL; i++) {
		D_("checking format [%d]: %s", s, pw_format[i]->name);
		res = pw_format[i]->test(b, title, s);
		if (res > 0) {
			return res;
		} else if (res == 0) {
			D_("format ok: %s\n", pw_format[i]->name);
			if (info != NULL) {
				memcpy(info->name, title, 21);
				strncpy(info->type, pw_format[i]->name,
							XMP_NAME_SIZE);
			}
			return 0;
		}
	}

	return -1;
}

void pw_read_title(unsigned char *b, char *t, int s)
{
	if (t == NULL) {
		return;
	}

	if (b == NULL) {
		*t = 0;
		return;
	}

	if (s > 20) {
		s = 20;
	}

	memcpy(t, b, s);
	t[s] = 0;
}

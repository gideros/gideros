#ifndef PROWIZ_H
#define PROWIZ_H

#include "stdio2.h"
#include "list.h"
#include "common.h"
#include "format.h"
#include "hio.h"

#define MIN_FILE_LENGHT 2048

#define PW_TEST_CHUNK   0x10000

#define MAGIC4(a,b,c,d) \
    (((uint32)(a)<<24)|((uint32)(b)<<16)|((uint32)(c)<<8)|(d))

#define PW_MOD_MAGIC	MAGIC4('M','.','K','.')

#define PW_REQUEST_DATA(s,n) \
	do { if ((s)<(n)) return ((n)-(s)); } while (0)

/*
 * depackb() and depackf() perform the same action reading the packed
 * module from a buffer or a file. We're supporting both protocols to
 * to avoid rewriting Asle's functions.
 */

struct pw_format {
	char *name;
	int (*test)(uint8 *, char *, int);
	int (*depack)(HIO_HANDLE *, FILE *);
	struct list_head list;
};

int pw_wizardry(HIO_HANDLE *, FILE *, char **);
int pw_move_data(FILE *, HIO_HANDLE *, int);
int pw_write_zero(FILE *, int);
/* int pw_enable(char *, int); */
int pw_check(unsigned char *, int, struct xmp_test_info *);
void pw_read_title(unsigned char *, char *, int);

extern const uint8 ptk_table[37][2];
extern const short tun_table[16][36];

extern const struct pw_format pw_ac1d;
extern const struct pw_format pw_crb;
extern const struct pw_format pw_di;
extern const struct pw_format pw_eu;
extern const struct pw_format pw_emod;
extern const struct pw_format pw_fcm;
extern const struct pw_format pw_fchs;
extern const struct pw_format pw_fuzz;
extern const struct pw_format pw_gmc;
extern const struct pw_format pw_hrt;
extern const struct pw_format pw_kris;
extern const struct pw_format pw_ksm;
extern const struct pw_format pw_mp_id;
extern const struct pw_format pw_mp_noid;
extern const struct pw_format pw_np1;
extern const struct pw_format pw_np2;
extern const struct pw_format pw_np3;
extern const struct pw_format pw_nru;
extern const struct pw_format pw_ntp;
extern const struct pw_format pw_p01;
extern const struct pw_format pw_p10c;
extern const struct pw_format pw_p18a;
extern const struct pw_format pw_p20;
extern const struct pw_format pw_p4x;
extern const struct pw_format pw_p50a;
extern const struct pw_format pw_p60a;
extern const struct pw_format pw_p61a;
extern const struct pw_format pw_pha;
extern const struct pw_format pw_pp21;
extern const struct pw_format pw_pp30;
extern const struct pw_format pw_pru1;
extern const struct pw_format pw_pru2;
extern const struct pw_format pw_skyt;
extern const struct pw_format pw_starpack;
extern const struct pw_format pw_stim;
extern const struct pw_format pw_tdd;
extern const struct pw_format pw_titanics;
extern const struct pw_format pw_tp3;
extern const struct pw_format pw_unic_emptyid;
extern const struct pw_format pw_unic_id;
extern const struct pw_format pw_unic_noid;
extern const struct pw_format pw_unic2;
extern const struct pw_format pw_wn;
extern const struct pw_format pw_xann;
extern const struct pw_format pw_zen;

#endif

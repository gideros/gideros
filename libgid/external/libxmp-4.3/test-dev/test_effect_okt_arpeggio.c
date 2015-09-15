#include "test.h"
#include <math.h>
#include "../src/effects.h"


/*
Periodtable for Tuning 0, Normal
  C-1 to B-1 : 856,808,762,720,678,640,604,570,538,508,480,453
  C-2 to B-2 : 428,404,381,360,339,320,302,285,269,254,240,226
  C-3 to B-3 : 214,202,190,180,170,160,151,143,135,127,120,113

Amiga limits: 907 to 108
*/

#define PERIOD ((int)round(1.0 * info.channel_info[0].period / 4096))

/* Get period from note */
static int note_to_period(int n)
{
        return round (13694.0 / pow(2, (double)n / 12));
}

static void check_arpeggio_okt3(xmp_context opaque, int note, int val, int spd)
{
	struct xmp_frame_info info;
	int i;
	int a1 = val >> 4;
	int a2 = val & 0x0f;
	int arp[20];
	char error[60];

	for (i = 0; i < 12; ) {
		arp[i++] = note_to_period(note - a1);
		arp[i++] = note_to_period(note);
		arp[i++] = note_to_period(note + a2);
	}

	for (i = 0; i < spd; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		snprintf(error, 60, "Oktalyzer arp3 0x%02x error", val);
		fail_unless(PERIOD == arp[i], error);
	}
}

static void check_arpeggio_okt4(xmp_context opaque, int note, int val, int spd)
{
	struct xmp_frame_info info;
	int i;
	int a1 = val >> 4;
	int a2 = val & 0x0f;
	int arp[20];
	char error[60];

	for (i = 0; i < 12; ) {
		arp[i++] = note_to_period(note);
		arp[i++] = note_to_period(note + a2);
		arp[i++] = note_to_period(note);
		arp[i++] = note_to_period(note - a1);
	}

	for (i = 0; i < spd; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		snprintf(error, 60, "Oktalyzer arp4 0x%02x error", val);
		fail_unless(PERIOD == arp[i], error);
	}
}

static void check_arpeggio_okt5(xmp_context opaque, int note, int val, int spd)
{
	struct xmp_frame_info info;
	int i;
	int a2 = val & 0x0f;
	int arp[20];
	char error[60];

	for (i = 0; i < 12; ) {
		arp[i++] = note_to_period(note + a2);
		arp[i++] = note_to_period(note + a2);
		arp[i++] = note_to_period(note);
	}

	for (i = 0; i < spd; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		snprintf(error, 60, "Oktalyzer arp5 0x%02x error", val);
		fail_unless(PERIOD == arp[i], error);
	}
}

TEST(test_effect_okt_arpeggio)
{
	xmp_context opaque;
	struct context_data *ctx;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	/* Oktalyzer arpeggio 3 */

	new_event(ctx, 0, 0, 0, 61, 1, 0, FX_OKT_ARP3, 0x00, 0, 0);
	new_event(ctx, 0, 1, 0, 61, 1, 0, FX_OKT_ARP3, 0x01, 0, 0);
	new_event(ctx, 0, 2, 0, 61, 1, 0, FX_OKT_ARP3, 0x05, 0, 0);
	new_event(ctx, 0, 3, 0, 61, 1, 0, FX_OKT_ARP3, 0x50, 0, 0);
	new_event(ctx, 0, 4, 0, 61, 1, 0, FX_OKT_ARP3, 0x35, 0, 0);

	xmp_start_player(opaque, 44100, 0);

	check_arpeggio_okt3(opaque, 60, 0x00, 6);
	check_arpeggio_okt3(opaque, 60, 0x01, 6);
	check_arpeggio_okt3(opaque, 60, 0x05, 6);
	check_arpeggio_okt3(opaque, 60, 0x50, 6);
	check_arpeggio_okt3(opaque, 60, 0x35, 6);

	/* Oktalyzer arpeggio 4 */

	new_event(ctx, 0, 0, 0, 61, 1, 0, FX_OKT_ARP4, 0x00, 0, 0);
	new_event(ctx, 0, 1, 0, 61, 1, 0, FX_OKT_ARP4, 0x01, 0, 0);
	new_event(ctx, 0, 2, 0, 61, 1, 0, FX_OKT_ARP4, 0x05, 0, 0);
	new_event(ctx, 0, 3, 0, 61, 1, 0, FX_OKT_ARP4, 0x50, 0, 0);
	new_event(ctx, 0, 4, 0, 61, 1, 0, FX_OKT_ARP4, 0x35, 0, 0);

	xmp_restart_module(opaque);

	check_arpeggio_okt4(opaque, 60, 0x00, 6);
	check_arpeggio_okt4(opaque, 60, 0x01, 6);
	check_arpeggio_okt4(opaque, 60, 0x05, 6);
	check_arpeggio_okt4(opaque, 60, 0x50, 6);
	check_arpeggio_okt4(opaque, 60, 0x35, 6);

	/* Oktalyzer arpeggio 5 */

	new_event(ctx, 0, 0, 0, 61, 1, 0, FX_OKT_ARP5, 0x00, 0, 0);
	new_event(ctx, 0, 1, 0, 61, 1, 0, FX_OKT_ARP5, 0x01, 0, 0);
	new_event(ctx, 0, 2, 0, 61, 1, 0, FX_OKT_ARP5, 0x05, 0, 0);
	new_event(ctx, 0, 3, 0, 61, 1, 0, FX_OKT_ARP5, 0x50, 0, 0);
	new_event(ctx, 0, 4, 0, 61, 1, 0, FX_OKT_ARP5, 0x35, 0, 0);

	xmp_restart_module(opaque);

	check_arpeggio_okt5(opaque, 60, 0x00, 6);
	check_arpeggio_okt5(opaque, 60, 0x01, 6);
	check_arpeggio_okt5(opaque, 60, 0x05, 6);
	check_arpeggio_okt5(opaque, 60, 0x50, 6);
	check_arpeggio_okt5(opaque, 60, 0x35, 6);
}
END_TEST

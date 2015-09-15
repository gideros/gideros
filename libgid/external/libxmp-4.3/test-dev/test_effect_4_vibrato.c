#include "test.h"
#include "../src/effects.h"
#include <math.h>

/*
Periodtable for Tuning 0, Normal
  C-1 to B-1 : 856,808,762,720,678,640,604,570,538,508,480,453
  C-2 to B-2 : 428,404,381,360,339,320,302,285,269,254,240,226
  C-3 to B-3 : 214,202,190,180,170,160,151,143,135,127,120,113

Amiga limits: 907 to 108
*/

#define PERIOD ((int)round(1.0 * info.channel_info[0].period / 4096))

static int vals[] = {
	143, 143, 144, 146, 147, 148,
	149, 149, 150, 150, 150, 149,
	254, 254, 257, 259, 261, 261,
	265, 265, 262, 258, 254, 250,
	453, 453, 459, 464, 467, 468,
	467, 467, 464, 459, 453, 447,
	808, 808, 819, 823, 819, 808,
	792, 792, 785, 792, 808, 824,
	1439, 1439, 1461, 1455, 1430, 1416,
	1428, 1428, 1465, 1455, 1416, 1418
};

static int vals2[] = {
	143, 144, 146, 147, 148, 149,
	150, 150, 150, 150, 148, 146,
	254, 257, 259, 261, 261, 261,
	262, 258, 254, 250, 246, 243,
	453, 459, 464, 467, 468, 467,
	464, 459, 453, 447, 442, 439,
	808, 819, 823, 819, 808, 797,
	785, 792, 808, 824, 831, 824,
	1439, 1461, 1455, 1430, 1416, 1430,
	1460, 1462, 1423, 1413, 1450, 1467 
};

static int vals3[] = {
	143, 143, 144, 145, 145, 146,
	146, 146, 146, 146, 145, 144,
	254, 255, 256, 257, 257, 257,
	258, 256, 254, 252, 250, 249,
	453, 456, 458, 460, 460, 460,
	458, 456, 453, 450, 448, 446,
	808, 813, 815, 813, 808, 803,
	797, 800, 808, 816, 819, 816,
	1439, 1450, 1447, 1435, 1428, 1435,
	1449, 1450, 1431, 1426, 1444, 1453
};


TEST(test_effect_4_vibrato)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	new_event(ctx, 0, 0, 0, 80, 1, 0, FX_VIBRATO, 0x24, 0, 0);
	new_event(ctx, 0, 1, 0, 0, 0, 0, FX_VIBRATO, 0x34, 0, 0);
	new_event(ctx, 0, 2, 0, 70, 0, 0, FX_VIBRATO, 0x44, 0, 0);
	new_event(ctx, 0, 3, 0, 0, 0, 0, FX_VIBRATO, 0x46, 0, 0);
	new_event(ctx, 0, 4, 0, 60, 0, 0, FX_VIBRATO, 0x48, 0, 0);
	new_event(ctx, 0, 5, 0, 0, 0, 0, FX_VIBRATO, 0x00, 0, 0);
	new_event(ctx, 0, 6, 0, 50, 0, 0, FX_VIBRATO, 0x88, 0, 0);
	new_event(ctx, 0, 7, 0, 0, 0, 0, FX_VIBRATO, 0x8c, 0, 0);
	new_event(ctx, 0, 8, 0, 40, 0, 0, FX_VIBRATO, 0xcc, 0, 0);
	new_event(ctx, 0, 9, 0, 0, 0, 0, FX_VIBRATO, 0xff, 0, 0);

	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 10 * 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(PERIOD == vals[i], "vibrato error");
	}

	/* check vibrato in all frames flag */

	xmp_restart_module(opaque);
	set_quirk(ctx, QUIRK_VIBALL, READ_EVENT_MOD);

	for (i = 0; i < 10 * 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(PERIOD == vals2[i], "vibrato error");
	}

	/* check deep vibrato flag */

	xmp_restart_module(opaque);
	set_quirk(ctx, QUIRK_VIBHALF, READ_EVENT_MOD);

	for (i = 0; i < 10 * 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(PERIOD == vals3[i], "half vibrato error");
	}
}
END_TEST

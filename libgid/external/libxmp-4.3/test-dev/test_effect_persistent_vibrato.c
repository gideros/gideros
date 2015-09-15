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
	143, 143, 154, 158, 154, 143,
	132, 132, 128, 132, 143, 154,
	158, 158, 154, 143, 132, 128,
	132, 132, 143, 154, 158, 154,
	143, 143, 143, 143, 143, 143,
	143, 143, 143, 143, 143, 143
};


TEST(test_effect_persistent_vibrato)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	new_event(ctx, 0, 0, 0, 80, 1, 0, FX_PER_VIBRATO, 0x88, 0, 0);
	new_event(ctx, 0, 4, 0, 80, 1, 0, FX_PER_VIBRATO, 0x40, 0, 0);

	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 6 * 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(PERIOD == vals[i], "vibrato error");
	}
}
END_TEST

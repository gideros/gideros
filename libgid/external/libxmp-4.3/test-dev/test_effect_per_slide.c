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

TEST(test_effect_per_slide)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i, j, k;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	/* Persistent portamento up */

	new_event(ctx, 0, 0, 0, 49, 1, 0, FX_PER_PORTA_UP, 2, 0, 0);
	new_event(ctx, 0, 60, 0, 0, 0, 0, FX_PER_PORTA_UP, 0, 0, 0);

	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 60; i++) {
		k = 856 - i * 10;
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(PERIOD == k, "slide up error (frame 0)");
		for (j = 0; j < 5; j++) {
			xmp_play_frame(opaque);
			xmp_get_frame_info(opaque, &info);
			fail_unless(PERIOD == k - j * 2, "slide up error");
		}
	}

	j--;
	xmp_play_frame(opaque);
	fail_unless(PERIOD == k - j * 2, "slide up error");

	/* Persistent portamento down */

	new_event(ctx, 0, 0, 0, 84, 1, 0, FX_PER_PORTA_DN, 2, 0, 0);
	new_event(ctx, 0, 60, 0, 0, 0, 0, FX_PER_PORTA_DN, 0, 0, 0);

	xmp_restart_module(opaque);

	for (i = 0; i < 60; i++) {
		k = 113 + i * 10;
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(PERIOD == k, "slide down error (frame 0)");
		for (j = 0; j < 5; j++) {
			xmp_play_frame(opaque);
			xmp_get_frame_info(opaque, &info);
			fail_unless(PERIOD == k + j * 2, "slide down error");
		}
	}

	j--;
	xmp_play_frame(opaque);
	fail_unless(PERIOD == k + j * 2, "slide down error");
}
END_TEST

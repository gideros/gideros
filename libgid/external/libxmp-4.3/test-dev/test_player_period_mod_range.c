#include "test.h"
#include <math.h>

/*
Periodtable for Tuning 0, Normal
  C-1 to B-1 : 856,808,762,720,678,640,604,570,538,508,480,453
  C-2 to B-2 : 428,404,381,360,339,320,302,285,269,254,240,226
  C-3 to B-3 : 214,202,190,180,170,160,151,143,135,127,120,113

Amiga limits: 907 to 108
*/

#define PERIOD ((int)round(1.0 * info.channel_info[0].period / 4096))

TEST(test_player_period_mod_range)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	xmp_start_player(opaque, 44100, 0);

	new_event(ctx, 0, 0, 0, 50, 1, 0, 0x0f, 1, 0, 0);
	new_event(ctx, 0, 1, 0, 49, 1, 0, 0, 0, 0, 0);
	new_event(ctx, 0, 2, 0, 48, 1, 0, 0, 0, 0, 0);
	new_event(ctx, 0, 3, 0, 83, 1, 0, 0, 0, 0, 0);
	new_event(ctx, 0, 4, 0, 84, 1, 0, 0, 0, 0, 0);
	new_event(ctx, 0, 5, 0, 85, 1, 0, 0, 0, 0, 0);

	/* test note limits */
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(PERIOD == 808, "Bad period");
	
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(PERIOD == 856, "Bad period");
	
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(PERIOD == 907, "Bad period");
	
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(PERIOD == 120, "Bad period");
	
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(PERIOD == 113, "Bad period");
	
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(PERIOD == 107, "Bad period");
	
	xmp_restart_module(opaque);

	/* test again with mod range on */
	set_quirk(ctx, QUIRK_MODRNG, READ_EVENT_MOD);

	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(PERIOD == 808, "Bad period");
	
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(PERIOD == 856, "Bad period");
	
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(PERIOD == 856, "Bad period");
	
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(PERIOD == 120, "Bad period");
	
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(PERIOD == 113, "Bad period");
	
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(PERIOD == 113, "Bad period");
	
	
	xmp_restart_module(opaque);

	/* test lower limit */
	new_event(ctx, 0, 0, 0, 49, 1, 0, 0x02, 1, 0, 0);
	for (i = 1; i < 20; i++)
		new_event(ctx, 0, i, 0,  0, 0, 0, 0x02, 1, 0, 0);
	for (i = 0; i < 20 * 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(PERIOD <= 907, "Bad lower limit");
	}

	xmp_restart_module(opaque);

	/* test upper limit */
	new_event(ctx, 0, 0, 0, 84, 1, 0, 0x01, 1, 0, 0);
	for (i = 1; i < 20; i++)
		new_event(ctx, 0, i, 0,  0, 0, 0, 0x01, 1, 0, 0);

	for (i = 0; i < 20 * 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(PERIOD >= 108, "Bad upper limit");
	}
}
END_TEST

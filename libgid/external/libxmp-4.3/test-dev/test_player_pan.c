#include "test.h"
#include "../src/effects.h"


TEST(test_player_pan)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i, pan0, pan1, pan2, pan3;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	/* set channel pan */

	for (i = 0; i < 4; i++) {
		ctx->m.mod.xxc[i].pan = 0x80;
	}

	/* set event pan */

	new_event(ctx, 0, 0, 0, 61, 1, 0, FX_SETPAN, 0x00, FX_SPEED, 3);
	new_event(ctx, 0, 0, 1, 61, 1, 0, FX_SETPAN, 0x40, 0, 0);
	new_event(ctx, 0, 0, 2, 61, 1, 0, FX_SETPAN, 0x80, 0, 0);
	new_event(ctx, 0, 0, 3, 61, 1, 0, FX_SETPAN, 0xc0, 0, 0);
	for (i = 1; i < 64; i++) {
		new_event(ctx, 0, i, 0, 61, 1, 0, FX_SETPAN, 0x00 + i, 0, 0);
		new_event(ctx, 0, i, 1, 61, 1, 0, FX_SETPAN, 0x40 + i, 0, 0);
		new_event(ctx, 0, i, 2, 61, 1, 0, FX_SETPAN, 0x80 + i, 0, 0);
		new_event(ctx, 0, i, 3, 61, 1, 0, FX_SETPAN, 0xc0 + i, 0, 0);
	}
	
	xmp_start_player(opaque, 44100, 0);

	/* set mix to 100% pan separation */
	xmp_set_player(opaque, XMP_PLAYER_MIX, 100);

	for (i = 0; i < 64; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);

		pan0 = info.channel_info[0].pan;
		pan1 = info.channel_info[1].pan;
		pan2 = info.channel_info[2].pan;
		pan3 = info.channel_info[3].pan;
		
		fail_unless(pan0 == 0x00 + i, "pan error in channel 0");
		fail_unless(pan1 == 0x40 + i, "pan error in channel 1");
		fail_unless(pan2 == 0x80 + i, "pan error in channel 2");
		fail_unless(pan3 == 0xc0 + i, "pan error in channel 3");

		xmp_play_frame(opaque);
		xmp_play_frame(opaque);
	}
}
END_TEST

#include "test.h"
#include "../src/effects.h"

static int vals[] = {
	0, 0, 0,
	0, 0, 0,
	0, 0, 5,
	10, 10, 8,
	6, 7, 7,
	7, 5, 5,
	255, 255, 255,
	255, 255, 255
};

TEST(test_effect_it_panslide)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	/* set channel pan */

	for (i = 0; i < 4; i++) {
		ctx->m.mod.xxc[i].pan = 0x80;
	}

	new_event(ctx, 0, 0, 0, 61, 1, 0, FX_SETPAN, 0x00, FX_SPEED, 3);
	new_event(ctx, 0, 1, 0, 0, 0, 0, FX_IT_PANSLIDE, 0x10, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0, FX_IT_PANSLIDE, 0x05, 0, 0);
	new_event(ctx, 0, 3, 0, 0, 0, 0, FX_IT_PANSLIDE, 0x20, 0, 0);
	new_event(ctx, 0, 4, 0, 0, 0, 0, FX_IT_PANSLIDE, 0xf1, 0, 0);
	new_event(ctx, 0, 5, 0, 0, 0, 0, FX_IT_PANSLIDE, 0x2f, 0, 0);
	new_event(ctx, 0, 6, 0, 61, 1, 0, FX_SETPAN, 0xff, 0, 0);
	new_event(ctx, 0, 7, 0, 0, 0, 0, FX_IT_PANSLIDE, 0x01, 0, 0);

	xmp_start_player(opaque, 44100, 0);

	/* set mix to 100% pan separation */

	xmp_set_player(opaque, XMP_PLAYER_MIX, 100);

	for (i = 0; i < 8 * 3; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].pan == vals[i], "pan error");
	}
}
END_TEST

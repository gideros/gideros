#include "test.h"
#include "../src/effects.h"


static int vals[] = {
	128, 173, 212, 238, 247, 238,
	212, 173, 128, 83, 44, 18,
	9, 18, 44, 83, 128, 173,
	212, 238, 247, 238, 212, 173,

	128, 106, 97, 106, 128, 150,
	159, 150, 128, 106, 97, 106,
	128, 128, 128, 128, 128, 128,
	128, 150, 159, 150, 128, 106
};

TEST(test_effect_panbrello)
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

	/* set event pan */

	new_event(ctx, 0, 0, 0, 61, 1, 0, FX_PANBRELLO, 0x4f, 0, 0);
	new_event(ctx, 0, 1, 0, 0, 0, 0, FX_PANBRELLO, 0x00, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0, FX_PANBRELLO, 0x00, 0, 0);
	new_event(ctx, 0, 3, 0, 0, 0, 0, FX_PANBRELLO, 0x00, 0, 0);
	new_event(ctx, 0, 4, 0, 0, 0, 0, FX_PANBRELLO, 0x84, 0, 0);
	new_event(ctx, 0, 5, 0, 0, 0, 0, FX_PANBRELLO, 0x00, 0, 0);
	new_event(ctx, 0, 6, 0, 0, 0, 0, 0, 0x00, 0, 0);
	new_event(ctx, 0, 7, 0, 0, 0, 0, FX_PANBRELLO, 0x00, 0, 0);
	
	xmp_start_player(opaque, 44100, 0);

	/* set mix to 100% pan separation */

	xmp_set_player(opaque, XMP_PLAYER_MIX, 100);

	for (i = 0; i < 48; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);

		fail_unless(info.channel_info[0].pan == vals[i], "pan error");
	}
}
END_TEST

#include "test.h"
#include "../src/effects.h"


static int vals[] = {
	64, 64, 62, 60,		/* down 2 */
	58, 58, 56, 54,		/* memory */
	52, 52, 53, 54,		/* up 1 */
	55, 55, 56, 57,		/* memory */
	63, 63, 63, 63,		/* set 63 */
	63, 63, 64, 64,		/* up 1 */
	1, 1, 1, 1,		/* set 1 */
	1, 1, 0, 0,		/* down 1 */
	10, 10, 10, 10,		/* set 10 */
	10, 10, 25, 40,		/* slide 0xf2 */
	55, 55, 56, 57		/* slide 0x1f */
};

static int vals_fine[] = {
	64, 64, 62, 60,		/* down 2 */
	58, 58, 56, 54,		/* memory */
	52, 52, 53, 54,		/* up 1 */
	55, 55, 56, 57,		/* memory */
	63, 63, 63, 63,		/* set 63 */
	63, 63, 64, 64,		/* up 1 */
	1, 1, 1, 1,		/* set 1 */
	1, 1, 0, 0,		/* down 1 */
	10, 10, 10, 10,		/* set 10 */
	10, 8, 8, 8,		/* fine slide down 2 */
	8, 9, 9, 9		/* fine slide up 1 */
};


TEST(test_effect_track_volslide)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	/* slide down & up with memory */
	new_event(ctx, 0, 0, 0, 49, 1, 0, FX_TRK_VSLIDE, 0x02, FX_SPEED, 4);
	new_event(ctx, 0, 1, 0, 0, 0, 0, FX_TRK_VSLIDE, 0x00, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0, FX_TRK_VSLIDE, 0x10, 0, 0);
	new_event(ctx, 0, 3, 0, 0, 0, 0, FX_TRK_VSLIDE, 0x00, 0, 0);

	/* limits */
	new_event(ctx, 0, 4, 0, 0, 0, 0, FX_TRK_VOL, 0x3f, 0, 0);
	new_event(ctx, 0, 5, 0, 0, 0, 0, FX_TRK_VSLIDE, 0x10, 0, 0);
	new_event(ctx, 0, 6, 0, 0, 0, 0, FX_TRK_VOL, 0x01, 0, 0);
	new_event(ctx, 0, 7, 0, 0, 0, 0, FX_TRK_VSLIDE, 0x01, 0, 0);

	/* fine effects */
	new_event(ctx, 0, 8, 0, 0, 0, 0, FX_TRK_VOL, 0x0a, 0, 0);
	new_event(ctx, 0, 9, 0, 0, 0, 0, FX_TRK_VSLIDE, 0xf2, 0, 0);
	new_event(ctx, 0, 10, 0, 0, 0, 0, FX_TRK_VSLIDE, 0x1f, 0, 0);

	/* play it */
	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 11 * 4; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].volume == vals[i], "volume slide error");

	}

	/* again with fine effects */
	set_quirk(ctx, QUIRK_FINEFX, READ_EVENT_MOD);
	xmp_restart_module(opaque);

	for (i = 0; i < 11 * 4; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].volume == vals_fine[i], "volume slide error");
	}
}
END_TEST

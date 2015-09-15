#include "test.h"
#include "../src/effects.h"

static int vals[] = {
	80, 80, 80,	/* set tempo */
	80, 78, 76,	/* slide down 2 */
	76, 77, 78,	/* slide up 1 */
	78, 78, 78,	/* nothing */
	32, 32, 32,	/* set as 0x20 */
	32, 32, 32,	/* slide down */
	255, 255, 255,	/* set as 0xff */
	255, 255, 255	/* slide up */
};

TEST(test_effect_it_bpm)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	new_event(ctx, 0, 0, 0, 0, 0, 0, FX_IT_BPM, 0x50, FX_SPEED, 3);
	new_event(ctx, 0, 1, 0, 0, 0, 0, FX_IT_BPM, 0x02, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0, FX_IT_BPM, 0x11, 0, 0);
	new_event(ctx, 0, 4, 0, 0, 0, 0, FX_IT_BPM, 0x20, 0, 0);
	new_event(ctx, 0, 5, 0, 0, 0, 0, FX_IT_BPM, 0x01, 0, 0);
	new_event(ctx, 0, 6, 0, 0, 0, 0, FX_IT_BPM, 0xff, 0, 0);
	new_event(ctx, 0, 7, 0, 0, 0, 0, FX_IT_BPM, 0x11, 0, 0);

	scan_sequences(ctx);

	xmp_start_player(opaque, 44100, 0);

	xmp_get_frame_info(opaque, &info);
	fail_unless(info.total_time == 4440, "total time error");

	for (i = 0; i < 8 * 3; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.bpm == vals[i], "tempo setting error");
	}
}
END_TEST

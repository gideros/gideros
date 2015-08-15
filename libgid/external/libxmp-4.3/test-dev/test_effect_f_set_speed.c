#include "test.h"
#include "../src/effects.h"

static int vals[] = {
	125, 3, 125, 3, 125, 3,		/* set speed to 0x03 */
	125, 31, 125, 31, 125, 31,
	125, 31, 125, 31, 125, 31,
	125, 31, 125, 31, 125, 31,
	125, 31, 125, 31, 125, 31,
	125, 31, 125, 31, 125, 31,
	125, 31, 125, 31, 125, 31,
	125, 31, 125, 31, 125, 31,
	125, 31, 125, 31, 125, 31,
	125, 31, 125, 31, 125, 31,
	125, 31, 125, 31, 125, 31,
	125, 31,			/* set speed to 0x1f */
	125, 2, 125, 2,			/* set speed to 0x02 */
	32, 2, 32, 2,			/* set speed to 0x20 */
	128, 2, 128, 2			/* set speed to 0x80 */
};

TEST(test_effect_f_set_speed)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	new_event(ctx, 0, 0, 0, 0, 0, 0, FX_SPEED, 0x03, 0, 0);
	new_event(ctx, 0, 1, 0, 0, 0, 0, FX_SPEED, 0x1f, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0, FX_SPEED, 0x02, 0, 0);
	new_event(ctx, 0, 3, 0, 0, 0, 0, FX_SPEED, 0x20, 0, 0);
	new_event(ctx, 0, 4, 0, 0, 0, 0, FX_SPEED, 0x80, 0, 0);

	scan_sequences(ctx);

	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < (3 + 0x1f + 3 * 2); i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.total_time == 5720, "total time error");
		fail_unless(info.bpm == vals[i * 2], "tempo setting error");
		fail_unless(info.speed == vals[i * 2 + 1], "speed setting error");
	}
}
END_TEST

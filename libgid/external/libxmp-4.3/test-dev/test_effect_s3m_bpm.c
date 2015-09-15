#include "test.h"
#include "../src/effects.h"

static int vals[] = {
	80, 80, 80,	/* set tempo */
	20, 20, 20,	/* set tempo 0x02 */
	20, 20, 20,	/* set tempo 0x11 */
	20, 20, 20,	/* nothing */
	32, 32, 32,	/* set tempo 0x20 */
	20, 20, 20,	/* set tempo 0x01 */
	255, 255, 255,	/* set tempo 0xff */
	20, 20, 20	/* set tempo 0x11 */
};

TEST(test_effect_s3m_bpm)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	new_event(ctx, 0, 0, 0, 0, 0, 0, FX_S3M_BPM, 0x50, FX_SPEED, 3);
	new_event(ctx, 0, 1, 0, 0, 0, 0, FX_S3M_BPM, 0x02, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0, FX_S3M_BPM, 0x11, 0, 0);
	new_event(ctx, 0, 4, 0, 0, 0, 0, FX_S3M_BPM, 0x20, 0, 0);
	new_event(ctx, 0, 5, 0, 0, 0, 0, FX_S3M_BPM, 0x01, 0, 0);
	new_event(ctx, 0, 6, 0, 0, 0, 0, FX_S3M_BPM, 0xff, 0, 0);
	new_event(ctx, 0, 7, 0, 0, 0, 0, FX_S3M_BPM, 0x11, 0, 0);

	scan_sequences(ctx);

	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 8 * 3; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.bpm == vals[i], "tempo setting error");
	}
	fail_unless(info.total_time == 4431, "total time error");
}
END_TEST

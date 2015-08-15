#include "test.h"
#include "../src/effects.h"

TEST(test_api_scan_module)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

	/* try to scan before loading */
	xmp_scan_module(opaque);

 	create_simple_module(ctx, 2, 2);

	new_event(ctx, 0, 0, 0, 0, 0, 0, FX_SPEED, 0x03, 0, 0);
	new_event(ctx, 0, 1, 0, 0, 0, 0, FX_SPEED, 0x1f, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0, FX_SPEED, 0x02, 0, 0);
	new_event(ctx, 0, 3, 0, 0, 0, 0, FX_SPEED, 0x20, 0, 0);
	new_event(ctx, 0, 4, 0, 0, 0, 0, FX_SPEED, 0x80, 0, 0);

	xmp_scan_module(opaque);

	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < (3 + 0x1f + 3 * 2); i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.total_time == 5720, "total time error");
	}
}
END_TEST

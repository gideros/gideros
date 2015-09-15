#include "test.h"
#include "../src/effects.h"


TEST(test_effect_persistent_vslide)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	int i, j, k;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);

	/* go up all the way */

	new_event(ctx, 0, 0, 0, 49, 1, 1, FX_PER_VSLD_UP, 1, 0, 0);

	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 80; i++) {
		k = i * 5;
		CLAMP(k, 0, 64);
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].volume == k, "volume slide error (frame 0)");
		for (j = 0; j < 5; j++) {
			xmp_play_frame(opaque);
			xmp_get_frame_info(opaque, &info);
			if (k + j > 64)
				fail_unless(info.channel_info[0].volume == 64, "volume slide up error");
			else {
				fail_unless(info.channel_info[0].volume == k + j, "volume slide up error");
			}
		}
	}

	/* go down all the way */

	new_event(ctx, 0, 0, 0, 84, 1, 65, FX_PER_VSLD_DN, 1, 0, 0);

	xmp_restart_module(opaque);

	for (i = 0; i < 80; i++) {
		k = 64 - i * 5;
		CLAMP(k, 0, 64);
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].volume == k, "volume slide error (frame 0)");
		for (j = 0; j < 5; j++) {
			xmp_play_frame(opaque);
			xmp_get_frame_info(opaque, &info);
			if (k - j < 0)
				fail_unless(info.channel_info[0].volume == 0, "volume slide up error");
			else {
				fail_unless(info.channel_info[0].volume == k - j, "volume slide up error");
			}
		}
	}

	/* go up just a little */

	new_event(ctx, 0, 0, 0, 49, 1, 1, FX_PER_VSLD_UP, 1, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0, FX_PER_VSLD_UP, 0, 0, 0);

	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 80; i++) {
		k = i * 5;
		CLAMP(k, 0, 10);
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].volume == k, "volume slide error (frame 0)");
		for (j = 0; j < 5; j++) {
			xmp_play_frame(opaque);
			xmp_get_frame_info(opaque, &info);
			if (k + j > 10)
				fail_unless(info.channel_info[0].volume == 10, "volume slide up error");
			else {
				fail_unless(info.channel_info[0].volume == k + j, "volume slide up error");
			}
		}
	}

	/* go down just a little */

	new_event(ctx, 0, 0, 0, 84, 1, 65, FX_PER_VSLD_DN, 1, 0, 0);
	new_event(ctx, 0, 2, 0, 0, 0, 0, FX_PER_VSLD_DN, 0, 0, 0);

	xmp_restart_module(opaque);

	for (i = 0; i < 80; i++) {
		k = 64 - i * 5;
		CLAMP(k, 54, 64);
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		fail_unless(info.channel_info[0].volume == k, "volume slide error (frame 0)");
		for (j = 0; j < 5; j++) {
			xmp_play_frame(opaque);
			xmp_get_frame_info(opaque, &info);
			if (k - j < 54)
				fail_unless(info.channel_info[0].volume == 54, "volume slide up error");
			else {
				fail_unless(info.channel_info[0].volume == k - j, "volume slide up error");
			}
		}
	}
}
END_TEST

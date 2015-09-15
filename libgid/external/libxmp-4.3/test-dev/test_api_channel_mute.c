#include "test.h"

TEST(test_api_channel_mute)
{
	xmp_context ctx;
	int ret;
	int i;

	ctx = xmp_create_context();
	xmp_load_module(ctx, "data/ode2ptk.mod");

	/* state check */
	ret = xmp_channel_mute(ctx, XMP_MAX_CHANNELS, 2);
	fail_unless(ret == -XMP_ERROR_STATE, "state check error");

	xmp_start_player(ctx, 8000, 0);

	/* invalid channel */
	ret = xmp_channel_mute(ctx, XMP_MAX_CHANNELS, 2);
	fail_unless(ret == -XMP_ERROR_INVALID, "invalid channel error");

	ret = xmp_channel_mute(ctx, -1, 2);
	fail_unless(ret == -XMP_ERROR_INVALID, "invalid channel error");

	for (i = 0; i < XMP_MAX_CHANNELS; i++) {
		/* query status */
		ret = xmp_channel_mute(ctx, i, -1);
		fail_unless(ret == 0, "mute status error");
	}

	for (i = 0; i < XMP_MAX_CHANNELS; i++) {
		if (i & 1) {
			/* mute */
			ret = xmp_channel_mute(ctx, i, 1);
			fail_unless(ret == 0, "previous status error");
			/* query */
			ret = xmp_channel_mute(ctx, i, -1);
			fail_unless(ret == 1, "mute channel error");
		}
		if (i < XMP_MAX_CHANNELS / 2) {
			/* toggle */
			ret = xmp_channel_mute(ctx, i, 2);
			if (i & 1) {
				fail_unless(ret == 1, "previous status error");
				/* query */
				ret = xmp_channel_mute(ctx, i, -1);
				fail_unless(ret == 0, "toggle channel error");
			} else {
				fail_unless(ret == 0, "previous status error");
				/* query */
				ret = xmp_channel_mute(ctx, i, -1);
				fail_unless(ret == 1, "toggle channel error");
			}
		} else {
			/* unmute */
			ret = xmp_channel_mute(ctx, i, 0);
			if (i & 1) {
				fail_unless(ret == 1, "previous status error");
			} else {
				fail_unless(ret == 0, "previous status error");
			}
			/* query */
			ret = xmp_channel_mute(ctx, i, -1);
			fail_unless(ret == 0, "unmute channel error");
		}
	}
}
END_TEST

#include "test.h"

TEST(test_api_channel_vol)
{
	xmp_context ctx;
	int ret;
	int i;

	ctx = xmp_create_context();
	xmp_load_module(ctx, "data/ode2ptk.mod");

	/* state check */
	ret = xmp_channel_vol(ctx, XMP_MAX_CHANNELS, 2);
	fail_unless(ret == -XMP_ERROR_STATE, "state check error");

	xmp_start_player(ctx, 8000, 0);

	/* invalid channel */
	ret = xmp_channel_vol(ctx, XMP_MAX_CHANNELS, 2);
	fail_unless(ret == -XMP_ERROR_INVALID, "invalid channel error");

	ret = xmp_channel_vol(ctx, -1, 2);
	fail_unless(ret == -XMP_ERROR_INVALID, "invalid channel error");

	for (i = 0; i < XMP_MAX_CHANNELS; i++) {
		/* query status */
		ret = xmp_channel_vol(ctx, i, -1);
		fail_unless(ret == 100, "volume error");
	}

	for (i = 0; i < XMP_MAX_CHANNELS; i++) {
		/* set */
		ret = xmp_channel_vol(ctx, i, i);
		fail_unless(ret == 100, "previous vol error");
		/* query */
		ret = xmp_channel_vol(ctx, i, -1);
		fail_unless(ret == i, "channel vol error");
	}
}
END_TEST

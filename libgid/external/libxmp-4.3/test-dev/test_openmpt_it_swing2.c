#include "test.h"

/*
 This module should remain completely silent, as the random variation is
 multiplied with the sample volume.
*/

TEST(test_openmpt_it_swing2)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci;
	int i;

	opaque = xmp_create_context();
	xmp_load_module(opaque, "openmpt/it/swing2.it");
	xmp_start_player(opaque, 8000, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		ci = &info.channel_info[0];
		fail_unless(ci->volume == 0, "volume not zero");
	}

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

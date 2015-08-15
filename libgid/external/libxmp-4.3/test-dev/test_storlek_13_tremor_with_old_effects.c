#include "test.h"

/*
13 - Tremor with old effects

Just when you think you've figured out tremor, guess what – it's even more
annoying. With Old Effects enabled, all non-zero values are incremented, so I40
with old effects means play for five ticks, and turn off for one; I51 means
play for six and off for two.

When this test is played correctly, both notes should play at exactly the same
intervals.
*/

TEST(test_storlek_13_tremor_with_old_effects)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci0, *ci1;;

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_13.it");
	xmp_start_player(opaque, 44100, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		ci0 = &info.channel_info[0];
		ci1 = &info.channel_info[1];

		fail_unless(ci0->volume == ci1->volume, "tremor error");
	}

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

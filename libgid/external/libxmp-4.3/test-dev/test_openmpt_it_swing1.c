#include "test.h"

/*
 Probably a very MPT-specific bug, as OpenMPT was adding the volume swing to
 the current volume, so even after setting the volume to 0, it was possible
 that you could hear the sample playing.
*/

TEST(test_openmpt_it_swing1)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci;
	int i;

	opaque = xmp_create_context();
	xmp_load_module(opaque, "openmpt/it/swing1.it");
	xmp_start_player(opaque, 8000, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		ci = &info.channel_info[0];
		if (info.row >= 2) {
			fail_unless(ci->volume == 0, "volume not zero");
		}
	}

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

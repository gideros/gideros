#include "test.h"

/*
 Of course you are not supposed to produce the same sequence of random number
 to fulfil this test case. I think this was just a test to explore which
 volume variables are actually affected by volume swing, and how it is applied.
*/

TEST(test_openmpt_it_swing3)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci;
	int values[64];
	int i;

	opaque = xmp_create_context();
	xmp_load_module(opaque, "openmpt/it/swing3.it");
	xmp_start_player(opaque, 8000, 0);
	xmp_set_player(opaque, XMP_PLAYER_MIX, 100);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		ci = &info.channel_info[0];
		if (info.frame == 0) {
			values[info.row] = ci->volume;
		}

	}

	/* Check pan randomness */
	fail_unless(check_randomness(values, 64, 5), "randomness error");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

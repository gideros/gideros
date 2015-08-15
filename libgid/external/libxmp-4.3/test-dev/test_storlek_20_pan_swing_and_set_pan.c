#include "test.h"

/*
 A couple of brief notes about instrument pan swing: All of the values are
 calculated with a range of 0-64. Values out of the 0-64 range are clipped.
 The swing simply defines the amount of variance from the current panning
 value.

 Given all of this, a pan swing value of 16 with a center-panned (32)
 instrument should produce values between 16 and 48; a swing of 32 with full
 right panning (64) will produce values between 0 -- technically -32 -- and 32.

 However, when a set panning effect is used along with a note, it should
 override the pan swing for that note.

 This test should play sets of notes with: Hard left panning Left-biased
 random panning Hard right panning Right-biased random panning Center panning
 with no swing Completely random values
*/

TEST(test_storlek_20_pan_swing_and_set_pan)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci;
	int values[64];
	int i;

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_20.it");
	xmp_start_player(opaque, 44100, 0);
	xmp_set_player(opaque, XMP_PLAYER_MIX, 100);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		ci = &info.channel_info[0];
		if (info.frame == 0) {
			values[info.row] = ci->pan;
		}

	}

	/* Check if pan values are kept in the empty rows */
	for (i = 0; i < 64; i += 2) {
		fail_unless(values[i] == values[i + 1], "pan value not kept");
	}

	/* Check if set pan values are used */
	for (i = 0; i < 8; i++) {
		fail_unless(values[i] == 0, "pan left not set");
	}
	for (i = 0; i < 8; i++) {
		fail_unless(values[16 + i] == 252, "pan right not set");
	}
	for (i = 0; i < 16; i++) {
		fail_unless(values[32 + i] == 128, "pan center not set");
	}

	/* Check pan randomness */
	fail_unless(check_randomness(values +  8,  8, 10), "randomness error");
	fail_unless(check_randomness(values + 24,  8, 10), "randomness error");
	fail_unless(check_randomness(values + 48, 16, 10), "randomness error");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

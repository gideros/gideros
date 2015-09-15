#include "test.h"

/*
11 - Infinite loop exploit

(Note: on this test, "fail" status is given for players which deadlock while
loading or calculating the duration, and is not based on actual playback
behavior. Incidentally, this will cause Impulse Tracker to freeze.)

This is a particularly evil pattern loop setup that exploits two possible
problems at the same time, and it will very likely cause any player to get
"stuck".

The first problem here is the duplicated loopback effect on the first channel;
the correct way to handle this is discussed in the previous test. The second
problem, and quite a bit more difficult to handle, is the seemingly strange
behavior after the third channel's loop plays once. What happens is the second
SB1 in the first channel "empties" its loopback counter, and when it reaches
the first SB1 again, the value is reset to 1. However, the second channel
hasn't looped yet, so playback returns to the first row. The next time around,
the second channel is done, but the first one needs to loop again — creating an
infinite loop situation. Even Impulse Tracker gets snagged by this.
*/

TEST(test_storlek_11_infinite_pattern_loop)
{
	xmp_context opaque;
	struct xmp_frame_info info;

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_11.it");
	xmp_start_player(opaque, 44100, 0);

	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &info);
	fail_unless(info.loop_count > 0, "loop not detected");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

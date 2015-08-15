#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
14 - Ping-pong loop and sample number

A lone sample number should not reset ping pong loop direction.

In this test, the sample should loop back and forth in both cases. If the
sample gets "stuck", check that the player is not touching the loop direction
unnecessarily.
*/

TEST(test_storlek_14_pingpong_loop_and_sample_number)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int position[100];
	int i = 0, j = 0;

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_14.it");
	xmp_start_player(opaque, 44100, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		if (info.row < 16) {
			position[i] = ci->position;
			i++;
		} else {
			fail_unless(ci->position == position[j], "position error");
			j++;
		}
	}

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

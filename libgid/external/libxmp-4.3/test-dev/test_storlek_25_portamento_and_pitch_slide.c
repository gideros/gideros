#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
25 - Portamento and pitch slide

The first two segments of this test should both play identically, with a slight
downward slide followed by a faster slide back up, and then remain at the
starting pitch. The third and fourth are provided primarily for reference for
possible misimplementations. (The third slides down and then continually rises;
the fourth slides down slightly and then plays a constant pitch.)

If the first and third segment are identical, then either Fx and Gxx aren't
sharing their values, or the Fx values aren't being multiplied (volume column
F1 should be equivalent to a "normal" F04).

If the first and fourth are identical, then the volume column is most likely
being processed prior to the effect column. Apparently, Impulse Tracker handles
the volume column effects last; note how the G04 needs to be placed on the next
row after the F1 in the second segment in order to mirror the behavior of the
first.
*/

TEST(test_storlek_25_portamento_and_pitch_slide)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int time, row, frame, chan, period, volume;
	char line[200];
	FILE *f;

	f = fopen("data/storlek_25.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_25.it");
	xmp_start_player(opaque, 44100, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		fgets(line, 200, f);
		sscanf(line, "%d %d %d %d %d %d",
			&time, &row, &frame, &chan, &period, &volume);

		fail_unless(info.time  == time,   "time mismatch");
		fail_unless(info.row   == row,    "row mismatch");
		fail_unless(info.frame == frame,  "frame mismatch");
		fail_unless(ci->period == period, "period mismatch");
		fail_unless(ci->volume == volume, "volume mismatch");
	}

	fgets(line, 200, f);
	fail_unless(feof(f), "not end of data file");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
08 - Out-of-range note delays

This test is to make sure note delay is handled correctly if the delay value
is out of range. The correct behavior is to act as if the entire row is empty.
Some players ignore the delay value and play the note on the first tick.

Oddly, Impulse Tracker does save the instrument number, even if the delay value
is out of range. I'm assuming this is a bug; nevertheless, if a player is going
to claim 100% IT compatibility, it needs to copy the bugs as well.

When played correctly, this should play the first three notes using the square
wave sample, with equal time between the start and end of each note, and the
last note should be played with the noise sample.
*/

TEST(test_storlek_08_out_of_range_delays)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int time, row, frame, chan, period, volume, ins;
	char line[200];
	FILE *f;

	f = fopen("data/storlek_08.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_08.it");
	xmp_start_player(opaque, 44100, 0);

	while (1) {

		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		fgets(line, 200, f);
		sscanf(line, "%d %d %d %d %d %d %d",
			&time, &row, &frame, &chan, &period, &volume, &ins);

		fail_unless(info.time  == time,   "time mismatch");
		fail_unless(info.row   == row,    "row mismatch");
		fail_unless(info.frame == frame,  "frame mismatch");
		fail_unless(ci->period == period, "period mismatch");
		fail_unless(ci->volume == volume, "volume mismatch");
		fail_unless(ci->instrument == ins, "instrument mismatch");
	}

	fgets(line, 200, f);
	fail_unless(feof(f), "not end of data file");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

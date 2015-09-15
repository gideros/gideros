#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
07 - Note cut with sample

Some players ignore sample numbers next to a note cut. When handled correctly,
this test should play a square wave, cut it, and then play the noise sample.

If this test is not handled correctly, make sure samples are checked regardless
of the note's value.
*/

TEST(test_storlek_07_note_cut_with_sample)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int time, row, frame, chan, period, volume, ins;
	char line[200];
	FILE *f;

	f = fopen("data/storlek_07.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_07.it");
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

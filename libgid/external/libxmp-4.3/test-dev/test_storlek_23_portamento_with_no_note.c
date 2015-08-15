#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
23 - Portamento with no note

Stray portamento effects with no target note should do nothing. Relatedly, a
portamento should clear the target note when it is reached.

The first section of this test should first play the same increasing tone three
times, with the last GFF effect not resetting the note to the base frequency;
the next part should play two rising tones at different pitches, and finish an
octave lower than it started.
*/

TEST(test_storlek_23_portamento_with_no_note)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int time, row, frame, chan, period, volume;
	char line[200];
	FILE *f;

	f = fopen("data/storlek_23.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_23.it");
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

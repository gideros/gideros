#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
05 - Gxx, fine slides, effect memory

(Note: make sure the Compatible Gxx flag is handled correctly before performing
this test.)

EFx and FFx are handled as fine slides even if the effect value was set by a
Gxx effect. If this test is played correctly, the pitch should bend downward a
full octave, and then up almost one semitone. If the note is bent way upward
(and possibly out of control, causing the note to stop), the player is not
handling the effect correctly.
*/

TEST(test_storlek_05_gxx_fine_slides_memory)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int time, row, frame, chan, period, volume;
	char line[200];
	FILE *f;

	f = fopen("data/storlek_05.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_05.it");
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

#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
02 - Arpeggio with no value

If this test plays correctly, both notes will sound the same, bending downward
smoothly. Incorrect (but perhaps acceptable, considering the unlikelihood of
this combination of pitch bend and a meaningless arpeggio) handling of the
arpeggio effect will result in a "stutter" on the second note, but the final
pitch should be the same for both notes. Really broken players will mangle the
pitch slide completely due to the arpeggio resetting the pitch on every third
tick.
*/

TEST(test_storlek_02_arpeggio_no_value)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int time, row, frame, chan, period, volume;
	char line[200];
	FILE *f;

	f = fopen("data/storlek_02.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_02.it");
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

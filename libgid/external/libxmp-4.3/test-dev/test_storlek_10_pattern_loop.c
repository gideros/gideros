#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
10 - Pattern loop

The pattern loop effect is quite complicated to handle, especially when dealing
with multiple channels. Possibly the most important fact to realize is that no
channel's pattern loop affects any other channel — each loop's processing
should be entirely contained to the channel the effect is in.

Another trouble spot that some players have is dealing correctly with strange
situations such as two consecutive loopback effects, e.g.:

000 | ... .. .. SB0
001 | ... .. .. SB1
002 | ... .. .. SB1
003 | ... .. .. .00

To prevent this from triggering an infinite loop, Impulse Tracker sets the
loopback point to the next row after the last SBx effect. This, the player flow
for the above fragment should be 0, 1, 0, 1, 2, 2, 3.

Another point to notice is when a loop should finish if two channels have SBx
effects on the same row:

000 | ... .. .. SB0 | ... .. .. SB0
001 | ... .. .. SB1 | ... .. .. SB2

What should happen here is the rows continue to loop as long as ANY of the
loopback counters are nonzero, or in other words, the least common multiple of
the total loop counts for each channel. In this case, the rows will play six
times — two loops for the first channel, and three for the second.

When correctly played, this test should produce a drum beat, slightly
syncopated. The entire riff repeats four times, and should sound the same all
four times.
*/

TEST(test_storlek_10_pattern_loop)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int time, row, frame, chan, period, volume, ins;
	char line[200];
	FILE *f;

	f = fopen("data/storlek_10.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_10.it");
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

#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
22 - Zero value for note cut and note delay

Impulse Tracker handles SD0 and SC0 as SD1 and SC1, respectively. (As a side
note, Scream Tracker 3 ignores notes with SD0 completely, and doesn't cut notes
at all with SC0.)

If these effects are handled correctly, the notes on the first row should
trigger simultaneously; the next pair of notes should not; and the final two
sets should both play identically and cut the notes after playing for one tick.
*/

TEST(test_storlek_22_zero_cut_and_delay)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_module_info minfo;
	struct xmp_channel_info *ci;
	int time, row, frame, chan, period, volume, ins;
	char line[200];
	FILE *f;
	int i;

	f = fopen("data/storlek_22.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_22.it");
	xmp_get_module_info(opaque, &minfo);

	xmp_start_player(opaque, 44100, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		for (i = 0; i < minfo.mod->chn; i++) {
			fgets(line, 200, f);
			sscanf(line, "%d %d %d %d %d %d %d", &time, &row,
				&frame, &chan, &period, &volume, &ins);

			ci = &info.channel_info[chan];

			fail_unless(info.time  == time,   "time mismatch");
			fail_unless(info.row   == row,    "row mismatch");
			fail_unless(info.frame == frame,  "frame mismatch");
			fail_unless(ci->period == period, "period mismatch");
			fail_unless(ci->volume == volume, "volume mismatch");
			fail_unless(ci->instrument == ins, "instrument mismatch");
		}
	}

	fgets(line, 200, f);
	fail_unless(feof(f), "not end of data file");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

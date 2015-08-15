#include "test.h"

TEST(test_player_med_synth)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci;
	int time, row, frame, chan, period, volume, ins, pan, smp;
	char line[200];
	FILE *f;
	int i, j;

	f = fopen("data/med_synth.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/Inertiaload-1.med");
	xmp_start_player(opaque, 44100, 0);

	for (i = 0; i < 500; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);

		for (j = 0; j < 4; j++) {
			ci = &info.channel_info[j];
			fgets(line, 200, f);
			sscanf(line, "%d %d %d %d %d %d %d %d %d",
					&time, &row, &frame, &chan, &period,
					&volume, &ins, &pan, &smp);

			fail_unless(info.time  == time,  "time mismatch");
			fail_unless(info.row   == row,   "row mismatch");
			fail_unless(info.frame == frame, "frame mismatch");
			fail_unless(ci->sample == smp,   "sample mismatch");
		}
	}

	fgets(line, 200, f);
	fail_unless(feof(f), "not end of data file");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

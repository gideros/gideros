#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


TEST(test_player_med_hold)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_module_info minfo;
	struct xmp_channel_info *ci;
	int time, row, frame, chan, period, volume, ins;
	char line[200];
	FILE *f;
	int i;

	f = fopen("data/med_hold.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/hold.med");
	xmp_get_module_info(opaque, &minfo);

	xmp_start_player(opaque, 44100, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

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

	fgets(line, 200, f);
	fail_unless(feof(f), "not end of data file");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

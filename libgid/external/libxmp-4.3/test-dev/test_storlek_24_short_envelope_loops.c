#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
24 - Short envelope loops

Envelope loops should include both of the loop points. Each instrument in this
test should play differently: first with no envelope, then a stuttering effect,
rapid left/right pan shift, and finally an arpeggio effect.
*/

TEST(test_storlek_24_short_envelope_loops)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int time, row, frame, chan, period, volume, ins, pan;
	char line[200];
	FILE *f;

	f = fopen("data/storlek_24.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_24.it");
	xmp_start_player(opaque, 44100, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		fgets(line, 200, f);
		sscanf(line, "%d %d %d %d %d %d %d %d", &time, &row,
			&frame, &chan, &period, &volume, &ins, &pan);

		fail_unless(info.time  == time,   "time mismatch");
		fail_unless(info.row   == row,    "row mismatch");
		fail_unless(info.frame == frame,  "frame mismatch");
		fail_unless(ci->period == period, "period mismatch");
		fail_unless(ci->volume == volume, "volume mismatch");
		fail_unless(ci->pan    == pan,    "Pan mismatch");
	}

	fgets(line, 200, f);
	fail_unless(feof(f), "not end of data file");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

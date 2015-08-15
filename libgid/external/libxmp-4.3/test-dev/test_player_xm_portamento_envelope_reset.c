#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* When a tone portamento effect is performed, we expect it to always
 * reset the envelope if the previous envelope already ended. If it
 * still didn't end, reset envelope in XM (and IT compatible GXX mode)
 * but not in standard IT mode
 */
TEST(test_player_xm_portamento_envelope_reset)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int time, row, frame, chan, period, volume, ins, pan;
	char line[200];
	FILE *f;

	f = fopen("data/xm_portamento_envelope_reset.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/xm_portamento_envelope_reset.xm");
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

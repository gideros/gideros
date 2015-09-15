#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
03 - Compatible Gxx off

Impulse Tracker links the effect memories for Exx, Fxx, and Gxx together if
"Compatible Gxx" in NOT enabled in the file header. In other formats,
portamento to note is entirely separate from pitch slide up/down. Several
players that claim to be IT-compatible do not check this flag, and always store
the last Gxx value separately.

When this test is played correctly, the first note will bend up, down, and back
up again, and the final set of notes should only slide partway down. Players
which do not correctly handle the Compatible Gxx flag will not perform the
final pitch slide in the first part, or will "snap" the final notes.
*/

TEST(test_storlek_03_compatible_gxx_off)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int time, row, frame, chan, period, volume;
	char line[200];
	FILE *f;

	f = fopen("data/storlek_03.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_03.it");
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

#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
09 - Sample change with no note

If a sample number is given without a note, Impulse Tracker will play the old
note with the new sample. This test should play the same beat twice, exactly
the same way both times. Players which do not handle sample changes correctly
will produce various interesting (but nonetheless incorrect!) results for the
second measure.
*/

struct data {
	int period;
	int volume;
	int instrument;
};

TEST(test_storlek_09_sample_change_no_note)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	struct data data[100];
	int i = 0, j = 0;

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_09.it");
	xmp_start_player(opaque, 44100, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		if (info.row < 16) {
			data[i].period = ci->period;
			data[i].volume = ci->volume;
			data[i].instrument = ci->instrument;
			i++;
		} else {
			fail_unless(ci->period == data[j].period, "period mismatch");
			fail_unless(ci->volume == data[j].volume, "volume mismatch");
			fail_unless(ci->instrument == data[j].instrument, "instrument mismatch");
			j++;
		}
	}

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

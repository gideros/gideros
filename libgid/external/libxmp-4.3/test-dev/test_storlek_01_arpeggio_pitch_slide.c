#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
01 - Arpeggio and pitch slide

Some players handle arpeggio incorrectly, storing and manipulating the original
pitch of the note instead of modifying the current pitch. While this has no
effect with "normal" uses of arpeggio, it causes strange problems when
combining arpeggio and a pitch bend.

When this test is played correctly, the first note will portamento upward,
arpeggiate for a few rows, and stay at the higher pitch. If this is played
incorrectly, it is most likely because the arpeggio effect is not checking the
current pitch of the note.

The second note should have a "stepping" effect. Make sure all three notes of
the arpeggio are being altered correctly by the volume-column pitch slide, not
just the base note. Also, the pitch after all effects should be approximately
the same as the third note.

Lastly, note that the arpeggio is set on the first row, prior to the note.
Certain players (notably, Modplug) erroneously ignore arpeggio values if no
note is playing.
*/

TEST(test_storlek_01_arpeggio_pitch_slide)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int time, row, frame, chan, period, volume;
	char line[200];
	FILE *f;

	f = fopen("data/storlek_01.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_01.it");
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

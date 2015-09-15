#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
15 - Retrigger

The retrig effect, in theory, should create an internal counter that decrements
from the "speed" value (the 'y' in Qxy) until it reaches zero, at which point
it resets. This timer should be unaffected by changes in the retrig speed –
that is, beginning a retrig and then changing the speed prior to the next
retrig point should not affect the timing of the next note. Additionally,
retrig is entirely independent of song speed, and the counter should reset when
new note is played.

As a side note, I would like to point out that the bassdrum sample uses a
silent loop at the end. This is a workaround for Impulse Tracker's behavior of
ignoring the retrig effect if no note is currently playing in the channel. Some
people seem to have misinterpreted this, coming to the conclusion that retrig
values greater than the song speed are ignored. However, this behavior is
rather inconvenient when dealing with very short samples. I encourage the
authors of other players to treat this behavior as a bug in Impulse Tracker's
playback engine and retrigger notes when the timer expires regardless of the
current state of the channel.
*/

TEST(test_storlek_15_retrigger)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci0 = &info.channel_info[0];
	struct xmp_channel_info *ci1 = &info.channel_info[1];
	int count = 0;

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_15.it");
	xmp_start_player(opaque, 44100, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		fail_unless((ci0->position == 0 && ci1->position == 0) ||
			    (ci0->position != 0 && ci1->position != 0),
			"retrigger error");
		count++;
	}

	fail_unless(count == 176, "short end");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
17 - Pattern row delay

Impulse Tracker has a slightly idiosyncratic way of handling the row delay
and note delay effects, and it's not very clearly explained in ITTECH.TXT.
In fact, row delay is mentioned briefly, albeit in a rather obscure manner,
and it's easy to glance over it. Considering this, it comes as no surprise
that this is one of the most strangely and hackishly implemented behaviors,
so much that pretty much everyone has written it off as an IT replayer bug.

In fact, it's not really a bug, just another artifact of the way Impulse
Tracker works. See, row delay doesn't touch the tick counter at all; it sets
its own counter, which the outer loop uses to repeat the row without
processing notes. The difference seems obscure until you try to replicate
the retriggering behavior of SEx on the same row with SDx. SDx, of course,
operates on all ticks except the first, and SEx repeats a row without
 handling the first tick, so the notes and first-tick effects only happen
once, while delayed notes play over and over as many times as the SEx effect
says.

Sound weird? Not really. As I said, it's even mentioned in ITTECH.TXT. Check
the flowchart under "Effect Info", and look at the words "Row counter". This
particular variable just says how many times the row plays (without replaying
notes) before moving to the next row. That's all. You just play the row more
than once.

This test plays sets of alternating bassdrum and snare, followed by a short
drum loop. In the first part, the bassdrum should play 2, 1, 7, 5, 1, and 5
times before each respective snare drum.
*/

TEST(test_storlek_17_pattern_row_delay)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_module_info minfo;
	struct xmp_channel_info *ci;
	int time, row, frame, chan, period, volume, ins, pan;
	char line[200];
	FILE *f;
	int i;

	f = fopen("data/storlek_17.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_17.it");
	xmp_get_module_info(opaque, &minfo);
	xmp_start_player(opaque, 44100, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		for (i = 0; i < minfo.mod->chn; i++) {
			fgets(line, 200, f);
			sscanf(line, "%d %d %d %d %d %d %d %d", &time, &row,
				&frame, &chan, &period, &volume, &ins, &pan);

			ci = &info.channel_info[chan];

			fail_unless(info.time  == time,   "time mismatch");
			fail_unless(info.row   == row,    "row mismatch");
			fail_unless(info.frame == frame,  "frame mismatch");
			fail_unless(ci->period == period, "period mismatch");
			fail_unless(ci->volume == volume, "volume mismatch");
			fail_unless(ci->pan    == pan,    "Pan mismatch");
		}
	}

	fgets(line, 200, f);
	fail_unless(feof(f), "not end of data file");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

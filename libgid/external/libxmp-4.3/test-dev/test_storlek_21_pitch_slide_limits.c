#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "../src/mixer.h"
#include "../src/virtual.h"

/*
21 - Pitch slide limits

Impulse Tracker always increases (or decreases, of course) the pitch of a
note with a pitch slide, with no limit on either the pitch of the note or
the amount of increment.

An odd side effect of this test is the harmonic strangeness resulting from
playing frequencies well above the Nyquist frequency. Different players
will seem to play the notes at wildly different pitches depending on the
interpolation algorithms and resampling rates used. Even changing the mixer
driver in Impulse Tracker will result in different apparent playback. The
important part of the behavior (and about the only thing that's fully
consistent) is that the frequency is changed at each step.
*/

TEST(test_storlek_21_pitch_slide_limits)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct module_data *m;
        struct player_data *p;
        struct mixer_voice *vi;
	struct xmp_frame_info fi;
	int time, row, frame, chan, period, note, ins, vol, pan, pos0;
	char line[200];
	FILE *f;
	int i, voc;

	f = fopen("data/storlek_21.data", "r");

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_21.it");

	ctx = (struct context_data *)opaque;
	m = &ctx->m;
	p = &ctx->p;

	xmp_start_player(opaque, 44100, 0);
	xmp_set_player(opaque, XMP_PLAYER_MIX, 100);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &fi);
		if (fi.loop_count > 0)
			break;

		for (i = 0; i < m->mod.chn; i++) {
			struct xmp_channel_info *ci = &fi.channel_info[i];

			voc = map_channel(p, i);
			if (voc < 0)
				continue;

			vi = &p->virt.voice_array[voc];

			fgets(line, 200, f);
			sscanf(line, "%d %d %d %d %d %d %d %d %d %d",
				&time, &row, &frame, &chan, &period,
				&note, &ins, &vol, &pan, &pos0);

			fail_unless(fi.time    == time,   "time mismatch");
			fail_unless(fi.row     == row,    "row mismatch");
			fail_unless(fi.frame   == frame,  "frame mismatch");
			fail_unless(ci->period == period, "period mismatch");
			fail_unless(vi->note   == note,   "note mismatch");
			fail_unless(vi->ins    == ins,    "instrument");
			fail_unless(vi->vol    == vol,    "volume mismatch");
			fail_unless(vi->pan    == pan,    "pan mismatch");
			fail_unless(vi->pos0   == pos0,   "position mismatch");
		}
		
	}

	fgets(line, 200, f);
	fail_unless(feof(f), "not end of data file");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

#include "test.h"

/*
19 - Random waveform

The random waveform for vibrato/tremolo/panbrello should not use a static
lookup table, but should be based on some form of random number generator.
Particularly, each playing channel should have a different value sequence.

Correct playback of this song should result in three stereo effects. It
might also be helpful to view the internal player variables in Impulse/Schism
Tracker's info page detail view (page up from the sample VU meters).
*/

static int val0[48 * 6], val1[48 * 6];

TEST(test_storlek_19_random_waveform)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *c0, *c1;
	int i, flag;

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_19.it");
	xmp_start_player(opaque, 44100, 0);

	/* play it */

	for (i = 0; i < 16 * 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);

		c0 = &info.channel_info[0];
		c1 = &info.channel_info[1];

		val0[i] = c0->volume;
		val1[i] = c1->volume;
	}

	for (; i < 32 * 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);

		c0 = &info.channel_info[0];
		c1 = &info.channel_info[1];

		val0[i] = c0->period;
		val1[i] = c1->period;
	}

	for (; i < 48 * 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);

		c0 = &info.channel_info[0];
		c1 = &info.channel_info[1];

		val0[i] = c0->pan;
		val1[i] = c1->pan;
	}

	/* now play it again and compare */
	xmp_restart_module(opaque);

	flag = 0;
	for (i = 0; i < 16 * 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);

		c0 = &info.channel_info[0];
		c1 = &info.channel_info[1];

		if (c0->volume == val0[i] && c1->volume == val1[i])
			flag++;
	}

	for (; i < 32 * 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);

		c0 = &info.channel_info[0];
		c1 = &info.channel_info[1];

		if (c0->period == val0[i] && c1->period == val1[i])
			flag++;
	}

	for (; i < 48 * 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);

		c0 = &info.channel_info[0];
		c1 = &info.channel_info[1];

		if (c0->pan == val0[i] && c1->pan == val1[i])
			flag++;
	}

	fail_unless(flag < 30, "random values error");

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

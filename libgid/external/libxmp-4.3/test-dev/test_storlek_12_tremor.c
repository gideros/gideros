#include "test.h"

/*
12 - Tremor effect

Like many other effects, tremor has an effect memory. This memory stores both
the "on" AND "off" values at once — they are not saved independently.

Also, when tremor is given a zero on- or off-value, that value is handled as
one tick instead. That is, I03 plays the same way as I13: play for one tick,
off for three.

Another potential snag is what happens when the note is "off" when the row
changes and the new row does not have a tremor effect. In this case, the volume
is always restored to normal, and the next time the effect is used, the
off-tick counter picks up right where it left off.

Finally, the only time the current tremor counts are reset is when the playback
is interrupted. Otherwise, the only part of the player code that should even
touch these values is the tremor effect handler, and it only ever decreases
the values... well, until they hit zero, at that point they are obviously
reset; but also note, the reset is independent for the on-tick and off-tick
counters.

When this test is played correctly, both notes should play at exactly the same
intervals.
*/

TEST(test_storlek_12_tremor)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci0, *ci1;;

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_12.it");
	xmp_start_player(opaque, 44100, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		ci0 = &info.channel_info[0];
		ci1 = &info.channel_info[1];

		fail_unless(ci0->volume == ci1->volume, "tremor error");
	}

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

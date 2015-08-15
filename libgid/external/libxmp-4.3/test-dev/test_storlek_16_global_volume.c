#include "test.h"

/*
16 - Global volume

This test checks the overall handling of the global volume (Vxx) and global
volume slide (Wxx) effects. If played properly, both notes should fade in from
silence to full volume, back to silence, and then fade in and out a second
time. (Note that the volume should be fading to and from maximum, i.e. two W80
effects at speed 9 should change the volume from 0 to 128.)

If the notes start at full volume instead of fading in, the V80 in channel 2 is
probably overriding the V00 in channel 3 on the first row. Similarly, if the
volume is suddenly cut on row 4, the V00 is probably incorrectly taking
precedence over the V80. Generally, for effects that alter global state, the
highest-numbered channel takes precedence.

Since two W80 effects at speed 9 raise the volume from zero to the maximum of
128, the V80 on row 3 should not have any effect on the volume.

Also, there are two spurious volume effects in channel 3, on rows 7 and 11.
Both of these effects should be ignored as out-of-range data, not clamped to
the maximum (or minimum!) volume.

Finally, the previous value for the global volume slide effect is saved per
channel. If the volume fades in and out, and does not fade back in, it is
almost certainly because separate global volume slide parameters are not stored
for each channel.
*/

TEST(test_storlek_16_global_volume)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	struct xmp_channel_info *ci = &info.channel_info[0];
	int vol[80];
	int i = 0;

	opaque = xmp_create_context();
	xmp_load_module(opaque, "data/storlek_16.it");
	xmp_start_player(opaque, 44100, 0);

	while (1) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		if (info.loop_count > 0)
			break;

		if (info.row < 8) {
			vol[i++] = ci->volume;
		} else {
			fail_unless(vol[i - 72] == ci->volume, "volume error");
			i++;
		}

	}

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

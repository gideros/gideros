#include "test.h"

/*
When a note is issued with no instrument after a keyoff event, it should
be played [if instrument has no envelope]

Last Battle.xm:

  00 D#4 02 .. ...
  01 ... .. .. ...
  02 C-4 .. .. ...
  03 ... .. .. ...
  04 === .. .. ...
  05 ... .. .. ...
  06 D#4 .. 31 ... <===
  07 ... .. .. ...
  08 C-4 .. .. ...
*/

TEST(test_player_ft2_note_noins_after_keyoff)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info fi;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

 	create_simple_module(ctx, 2, 2);
	set_instrument_volume(ctx, 0, 0, 64);
	new_event(ctx, 0, 0, 0, 60, 1, 0, 0x0f, 2, 0, 0);
	new_event(ctx, 0, 1, 0, XMP_KEY_OFF, 0,  0, 0, 0, 0, 0);
	new_event(ctx, 0, 2, 0, 50, 0, 20, 0, 0, 0, 0);
	set_quirk(ctx, QUIRKS_FT2, READ_EVENT_FT2);

	xmp_start_player(opaque, 44100, 0);

	/* Row 0 */
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &fi);
	fail_unless(fi.channel_info[0].note == 59, "set note");
	fail_unless(fi.channel_info[0].volume == 64, "set volume");
	xmp_play_frame(opaque);

	/* Row 1: key off */
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &fi);
	fail_unless(fi.channel_info[0].note == 59, "set note");
	fail_unless(fi.channel_info[0].volume == 0, "set volume");
	xmp_play_frame(opaque);

	/* Row 2: new note */
	xmp_play_frame(opaque);
	xmp_get_frame_info(opaque, &fi);
	fail_unless(fi.channel_info[0].note == 49, "set note");
	fail_unless(fi.channel_info[0].volume == 19, "set volume");
	xmp_play_frame(opaque);
}
END_TEST

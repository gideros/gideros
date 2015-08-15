#include "test.h"
#include "../src/mixer.h"
#include "../src/virtual.h"


/*

  C-5 39	-> valid instrument
  --- 30	-> valid instrument
  C-5 --	-> this note plays

  C-5 39	-> valid instrument
  --- 28	-> invalid instrument
  C-5 --	-> this note shouldn't play

*/

TEST(test_player_ft2_note_noins_after_invalid_ins)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct player_data *p;
	struct mixer_voice *vi;
	int voc;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	p = &ctx->p;

 	create_simple_module(ctx, 2, 2);
	set_instrument_volume(ctx, 0, 0, 22);
	set_instrument_volume(ctx, 1, 0, 33);
	new_event(ctx, 0, 0, 0, 60, 1, 44, 0x0f, 2, 0, 0);
	new_event(ctx, 0, 1, 0,  0, 2,  0, 0x00, 0, 0, 0);
	new_event(ctx, 0, 2, 0, 50, 0,  0, 0x00, 0, 0, 0);
	new_event(ctx, 0, 3, 0, 60, 1, 44, 0x00, 0, 0, 0);
	new_event(ctx, 0, 4, 0,  0, 3,  0, 0x00, 0, 0, 0);
	new_event(ctx, 0, 5, 0, 50, 0,  0, 0x00, 0, 0, 0);
	set_quirk(ctx, QUIRKS_FT2, READ_EVENT_FT2);

	xmp_start_player(opaque, 44100, 0);

	/* Row 0 - valid instrument */
	xmp_play_frame(opaque);

	voc = map_channel(p, 0);
	fail_unless(voc >= 0, "virtual map");
	vi = &p->virt.voice_array[voc];

	fail_unless(vi->note == 59, "set note");
	fail_unless(vi->ins  ==  0, "set instrument");
	fail_unless(vi->vol  == 43 * 16, "set volume");

	xmp_play_frame(opaque);

	/* Row 1 - valid instrument */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 59, "set note");
	fail_unless(vi->ins  ==  0, "not same instrument");
	fail_unless(vi->vol  !=  0, "cut sample");
	xmp_play_frame(opaque);

	/* Row 2 - no instrument (should play) */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 49, "set note");
	fail_unless(vi->ins  ==  1, "set instrument");
	fail_unless(vi->vol  !=  0, "cut sample");
	xmp_play_frame(opaque);

	/* Row 3 - valid instrument */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 59, "set note");
	fail_unless(vi->ins  ==  0, "set instrument");
	fail_unless(vi->vol  !=  0, "cut sample");
	xmp_play_frame(opaque);

	/* Row 4 - invalid instrument */
	xmp_play_frame(opaque);
	fail_unless(vi->vol  !=  0, "cut sample");
	xmp_play_frame(opaque);

	/* Row 5 - no instrument (shouldn't play) */
	xmp_play_frame(opaque);
	fail_unless(vi->vol  ==  0, "didn't cut sample");
	xmp_play_frame(opaque);

 
}
END_TEST

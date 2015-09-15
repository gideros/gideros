#include "test.h"
#include "../src/mixer.h"
#include "../src/virtual.h"


TEST(test_player_note_off_ft2)
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

	new_event(ctx, 0, 0, 0, 60, 1, 44, 0x0f, 2, 0, 0);
	new_event(ctx, 0, 1, 0, XMP_KEY_OFF, 0,  0, 0x00, 0, 0, 0);
	set_quirk(ctx, QUIRKS_FT2, READ_EVENT_FT2);
	set_instrument_fadeout(ctx, 0, 16000);

	/* Test: Without envelope in FT2 mode */
	xmp_start_player(opaque, 44100, 0);
	xmp_play_frame(opaque);

	voc = map_channel(p, 0);
	fail_unless(voc >= 0, "virtual map");
	vi = &p->virt.voice_array[voc];

	fail_unless(vi->note == 59, "set note");
	fail_unless(vi->ins  ==  0, "set instrument");
	fail_unless(vi->vol / 16 == 43, "set volume");
	fail_unless(vi->pos0 ==  0, "sample position");

	xmp_play_frame(opaque);

	/* Row 1: test keyoff */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 59, "not same note");
	fail_unless(vi->ins  ==  0, "not same instrument");
	fail_unless(vi->vol / 16 == 0, "didn't cut note");

	/* Test: With envelope in FT2 mode */
        set_instrument_envelope(ctx, 0, 0, 0, 32);
	set_instrument_envelope(ctx, 0, 1, 1, 32);
	set_instrument_envelope(ctx, 0, 2, 2, 64);
	set_instrument_envelope(ctx, 0, 3, 4, 0);
	set_instrument_envelope_sus(ctx, 0, 1);

	xmp_restart_module(opaque);
	xmp_play_frame(opaque);

	fail_unless(vi->note == 59, "set note");
	fail_unless(vi->ins  ==  0, "set instrument");
	fail_unless(vi->vol / 16 == 21, "envelope volume");
	fail_unless(vi->pos0 ==  0, "sample position");

	xmp_play_frame(opaque);

	/* Row 1: test keyoff */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 59, "not same note");
	fail_unless(vi->ins  ==  0, "not same instrument");
	fail_unless(vi->vol / 16 == 21, "didn't follow envelope + fadeout");

	xmp_play_frame(opaque);
	fail_unless(vi->vol / 16 == 32, "didn't follow envelope");
}
	
END_TEST

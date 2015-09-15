#include "test.h"
#include "../src/effects.h"
#include "../src/mixer.h"
#include "../src/virtual.h"


TEST(test_effect_set_nna_fade)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct player_data *p;
	struct mixer_voice *vi, *vi2;
	int i, voc;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	p = &ctx->p;

 	create_simple_module(ctx, 2, 2);
	set_instrument_volume(ctx, 0, 0, 22);
	set_instrument_volume(ctx, 1, 0, 33);
	set_instrument_fadeout(ctx, 0, 10000);

	new_event(ctx, 0, 0, 0, 60, 1, 44, 0x0f, 2, FX_IT_INSTFUNC, 0x06);
	new_event(ctx, 0, 1, 0, 50, 2,  0, 0x00, 0, 0, 0);
	set_quirk(ctx, QUIRKS_IT, READ_EVENT_IT);

	xmp_start_player(opaque, 44100, 0);

	/* Row 0 */
	xmp_play_frame(opaque);

	voc = map_channel(p, 0);
	fail_unless(voc >= 0, "virtual map");
	vi = &p->virt.voice_array[voc];

	fail_unless(vi->note == 59, "set note");
	fail_unless(vi->ins  ==  0, "set instrument");
	fail_unless(vi->vol / 16 == 43, "set volume");
	fail_unless(vi->pos0 ==  0, "sample position");

	xmp_play_frame(opaque);

	/* Row 1: new event to test NNA */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 59, "not same note");
	fail_unless(vi->ins  ==  0, "not same instrument");
	fail_unless(vi->vol / 16 == 36, "not fading out");
	fail_unless(vi->pos0 !=  0, "sample reset");

	/* Find virtual voice for channel 0 */
	for (i = 0; i < p->virt.maxvoc; i++) {
		if (p->virt.voice_array[i].chn == 0) {
			break;
		}
	}
	fail_unless(i != p->virt.maxvoc, "didn't virtual voice");

	/* New instrument in virtual channel. When NNA is set to OFF,
	 * the new instrument plays in a new voice and the old instrument
	 * escapes from the envelope sustain point (keyoff event), follows
	 * the rest of the envelope and resets the virtual channel
	 */
	vi2 = &p->virt.voice_array[i];
	fail_unless(vi2->ins  ==  1, "not new instrument");
	fail_unless(vi2->note == 49, "not new note");
	fail_unless(vi2->vol  == 33 * 16, "not new instrument volume");
	fail_unless(vi2->pos0 ==  0, "sample didn't reset");

	/* Fadeout */
	xmp_play_frame(opaque);
	fail_unless(vi->chn == 4, "didn't copy channel");
	fail_unless(vi->note == 59, "not same note");
	fail_unless(vi->ins  ==  0, "not same instrument");
	fail_unless(vi->vol / 16 == 29, "not fading out");
	fail_unless(vi->pos0 !=  0, "sample reset");

	xmp_play_frame(opaque);
	fail_unless(vi->chn == 4, "didn't copy channel");
	fail_unless(vi->note == 59, "not same note");
	fail_unless(vi->ins  ==  0, "not same instrument");
	fail_unless(vi->vol / 16 == 23, "not fading out");
	fail_unless(vi->pos0 !=  0, "sample reset");

	xmp_play_frame(opaque);
	fail_unless(vi->chn == 4, "didn't copy channel");
	fail_unless(vi->note == 59, "not same note");
	fail_unless(vi->ins  ==  0, "not same instrument");
	fail_unless(vi->vol / 16 == 16, "not fading out");
	fail_unless(vi->pos0 !=  0, "sample reset");
}
END_TEST

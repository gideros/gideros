#include "test.h"
#include "../src/mixer.h"
#include "../src/virtual.h"


TEST(test_player_nna_cont)
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
	set_instrument_nna(ctx, 0, 0, XMP_INST_NNA_CONT, XMP_INST_DCT_OFF,
							XMP_INST_DCA_CUT);
	new_event(ctx, 0, 0, 0, 60, 1, 44, 0x0f, 2, 0, 0);
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
	fail_unless(vi->vol  == 43 * 16, "set volume");
	fail_unless(vi->pos0 ==  0, "sample position");

	xmp_play_frame(opaque);

	/* Row 1: new event to test NNA */
	xmp_play_frame(opaque);
	fail_unless(vi->note == 59, "not same note");
	fail_unless(vi->ins  ==  0, "not same instrument");
	fail_unless(vi->vol  == 43 * 16, "not same volume");
	fail_unless(vi->pos0 !=  0, "sample reset");

	/* Find virtual voice for channel 0 */
	for (i = 0; i < p->virt.maxvoc; i++) {
		if (p->virt.voice_array[i].chn == 0) {
			break;
		}
	}
	fail_unless(i != p->virt.maxvoc, "didn't find virtual voice");

	/* New instrument in virtual channel. When NNA is set to CONT,
	 * the new instrument plays in a new virtual voice and the previous
	 * instrument stays playing in the original voice
	 */
	vi2 = &p->virt.voice_array[i];
	fail_unless(vi2->chn  ==  0, "not following channel");
	fail_unless(vi2->ins  ==  1, "not new instrument");
	fail_unless(vi2->note == 49, "not new note");
	fail_unless(vi2->vol  == 33 * 16, "not new instrument volume");
	fail_unless(vi2->pos0 ==  0, "sample didn't reset");
	xmp_play_frame(opaque);

	/* Some rows later... check if volume still the same */
	for (i = 0; i < 20; i++) {
		xmp_play_frame(opaque);
		fail_unless(vi->chn  == 4, "didn't copy channel");
		fail_unless(vi->note == 59, "first note: not same note");
		fail_unless(vi->ins  ==  0, "first note: not same instrument");
		fail_unless(vi->vol  == 43 * 16, "first note: not same volume");
	}
}
END_TEST

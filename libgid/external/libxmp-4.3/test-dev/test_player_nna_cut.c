#include "test.h"
#include "../src/mixer.h"
#include "../src/virtual.h"


TEST(test_player_nna_cut)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct player_data *p;
	struct mixer_voice *vi;
	int i, voc;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	p = &ctx->p;

 	create_simple_module(ctx, 2, 2);
	set_instrument_volume(ctx, 0, 0, 22);
	set_instrument_volume(ctx, 1, 0, 33);
	set_instrument_nna(ctx, 1, 0, XMP_INST_NNA_CUT, XMP_INST_DCT_OFF,
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
	fail_unless(vi->ins  ==  1, "not new instrument");
	fail_unless(vi->note == 49, "not new note");
	fail_unless(vi->vol  == 33 * 16, "not new instrument volume");
	fail_unless(vi->pos0 ==  0, "sample didn't reset");

	for (i = 0; i < p->virt.maxvoc; i++) {
		struct mixer_voice *vi = &p->virt.voice_array[i];
		if (i != 0 && vi->root == 0) {
			break;
		}
	}
	fail_unless(i == p->virt.maxvoc, "used virtual voice");

	xmp_play_frame(opaque);
}
END_TEST

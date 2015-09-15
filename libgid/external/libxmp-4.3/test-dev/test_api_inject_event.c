#include "test.h"
#include "../src/mixer.h"
#include "../src/virtual.h"

TEST(test_api_inject_event)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct player_data *p;
	struct mixer_voice *vi;
	struct xmp_event event = { 60, 2, 40, 0xf, 3, 0, 0 };
	int voc;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	p = &ctx->p;

 	create_simple_module(ctx, 2, 2);

	xmp_start_player(opaque, 44100, 0);
	xmp_play_frame(opaque);

	xmp_inject_event(opaque, 3, &event);
	xmp_play_frame(opaque);
	voc = map_channel(p, 3);
	fail_unless(voc >= 0, "virtual map");
	vi = &p->virt.voice_array[voc];

	fail_unless(vi->note == 59, "set note");
	fail_unless(vi->ins  == 1 , "set instrument");
	fail_unless(vi->vol  == 39 * 16, "set volume");
	fail_unless(p->speed == 3 , "set effect");
	fail_unless(vi->pos0 == 0 , "sample position");
}
END_TEST

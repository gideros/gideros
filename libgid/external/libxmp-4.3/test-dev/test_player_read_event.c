#include "test.h"
#include "../src/mixer.h"
#include "../src/virtual.h"

TEST(test_player_read_event)
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
	new_event(ctx, 0, 0, 0, 60, 2, 40, 0x0f, 3, 0, 0);
	new_event(ctx, 0, 1, 0, 61, 1,  0, 0x00, 0, 0, 0);

	xmp_start_player(opaque, 44100, 0);
	xmp_play_frame(opaque);

	voc = map_channel(p, 0);
	fail_unless(voc >= 0, "virtual map");
	vi = &p->virt.voice_array[voc];

	fail_unless(vi->note == 59, "set note");
	fail_unless(vi->ins  == 1 , "set instrument");
	fail_unless(vi->vol  == 39 * 16, "set volume");
	fail_unless(p->speed == 3 , "set effect");
	fail_unless(vi->pos0 == 0 , "sample position");

	xmp_play_frame(opaque);
	fail_unless(vi->note == 59, "set note");
	fail_unless(vi->ins  == 1 , "set instrument");
	fail_unless(vi->vol  == 39 * 16, "set volume");
	fail_unless(vi->pos0 != 0 , "sample position");

	xmp_play_frame(opaque);
	fail_unless(vi->note == 59, "set note");
	fail_unless(vi->ins  == 1 , "set instrument");
	fail_unless(vi->vol  == 39 * 16, "set volume");
	fail_unless(vi->pos0 != 0 , "sample position");

	xmp_play_frame(opaque);
	fail_unless(vi->note == 60, "set note");
	fail_unless(vi->ins  == 0 , "set instrument");
	fail_unless(vi->vol  == 64 * 16, "set volume");
	fail_unless(vi->pos0 == 0 , "sample position");
}
END_TEST

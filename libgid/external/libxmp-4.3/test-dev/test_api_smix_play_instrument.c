#include "test.h"
#include "../src/mixer.h"
#include "../src/virtual.h"

TEST(test_api_smix_play_instrument)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct player_data *p;
	struct mixer_voice *vi;
	int voc, ret;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	p = &ctx->p;

	xmp_start_smix(opaque, 1, 2);

	ret = xmp_load_module(opaque, "data/mod.loving_is_easy.pp");
	fail_unless(ret == 0, "load module");

	/* play instrument before starting player */
	ret = xmp_smix_play_instrument(opaque, 2, 60, 64, 0);
	fail_unless(ret == -XMP_ERROR_STATE, "invalid state");

	xmp_start_player(opaque, 44100, 0);
	xmp_play_frame(opaque);

	/* play invalid instrument */
	ret = xmp_smix_play_instrument(opaque, 31, 60, 64, 0);
	fail_unless(ret == -XMP_ERROR_INVALID, "invalid instrument");

	/* play instrument in invalid channel */
	ret = xmp_smix_play_instrument(opaque, 31, 60, 64, 1);
	fail_unless(ret == -XMP_ERROR_INVALID, "invalid instrument");

	ret = xmp_smix_play_instrument(opaque, 2, 60, 64, 0);
	fail_unless(ret == 0, "play instrument");
	xmp_play_frame(opaque);

	voc = map_channel(p, 4);
	fail_unless(voc >= 0, "virtual map");
	vi = &p->virt.voice_array[voc];

	fail_unless(vi->note == 60, "set note");
	fail_unless(vi->ins  ==  2, "set instrument");
	fail_unless(vi->vol / 16 == 64, "set volume");
	fail_unless(vi->pos0 ==  0, "sample position");

	ret = xmp_smix_play_instrument(opaque, 3, 50, 40, 0);
	fail_unless(ret == 0, "play_sample");
	xmp_play_frame(opaque);

	fail_unless(vi->note == 50, "set note");
	fail_unless(vi->ins  ==  3, "set instrument");
	fail_unless(vi->vol / 16 == 40, "set volume");
	fail_unless(vi->pos0 ==  0, "sample position");

	xmp_release_module(opaque);
	xmp_end_smix(opaque);
}
END_TEST

#include "test.h"
#include "../src/mixer.h"
#include "../src/virtual.h"

TEST(test_api_smix_play_sample)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct player_data *p;
	struct mixer_voice *vi;
	int voc, ret;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	p = &ctx->p;

	ret = xmp_load_module(opaque, "data/mod.loving_is_easy.pp");
	fail_unless(ret == 0, "load module");

	xmp_start_smix(opaque, 1, 2);

	ret = xmp_smix_load_sample(opaque, 0, "data/blip.wav");
	fail_unless(ret == 0, "load sample 0");
        ret = xmp_smix_load_sample(opaque, 1, "data/buzz.wav");
	fail_unless(ret == 0, "load sample 1");

	/* play sample before starting player */
	ret = xmp_smix_play_sample(opaque, 2, 60, 64, 0);
	fail_unless(ret == -XMP_ERROR_STATE, "invalid state");

	xmp_start_player(opaque, 44100, 0);
	xmp_play_frame(opaque);

	/* play invalid sample */
	ret = xmp_smix_play_sample(opaque, 2, 60, 64, 0);
	fail_unless(ret == -XMP_ERROR_INVALID, "invalid sample");

	/* play sample in invalid channel */
	ret = xmp_smix_play_sample(opaque, 0, 60, 64, 1);
	fail_unless(ret == -XMP_ERROR_INVALID, "invalid channel");

	ret = xmp_smix_play_sample(opaque, 0, 60, 64, 0);
	fail_unless(ret == 0, "play sample");
	xmp_play_frame(opaque);

	voc = map_channel(p, 4);
	fail_unless(voc >= 0, "virtual map");
	vi = &p->virt.voice_array[voc];

	fail_unless(vi->note - ctx->smix.xxi[0].sub[0].xpo == 60, "set note");
	fail_unless(vi->ins  ==  31, "set instrument");
	fail_unless(vi->vol / 16 == 64, "set volume");
	fail_unless(vi->pos0 ==  0, "sample position");

	ret = xmp_smix_play_sample(opaque, 1, 50, 40, 0);
	fail_unless(ret == 0, "play_sample");
	xmp_play_frame(opaque);

	fail_unless(vi->note - ctx->smix.xxi[1].sub[0].xpo == 50, "set note");
	fail_unless(vi->ins  ==  32, "set instrument");
	fail_unless(vi->vol / 16 == 40, "set volume");
	fail_unless(vi->pos0 ==  0, "sample position");

	xmp_smix_release_sample(opaque, 0);
	xmp_smix_release_sample(opaque, 1);

	xmp_end_smix(opaque);
}
END_TEST

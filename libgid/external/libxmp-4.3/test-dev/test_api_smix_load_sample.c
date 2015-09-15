#include <errno.h>
#include "test.h"

TEST(test_api_smix_load_sample)
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

	/* try to load sample before initializing */
	ret = xmp_smix_load_sample(opaque, 0, "data/blip.wav");
	fail_unless(ret == -XMP_ERROR_INVALID, "load sample before init");

	xmp_start_smix(opaque, 1, 2);

	/* try to load in invalid slot */
	ret = xmp_smix_load_sample(opaque, 2, "data/blip.wav");
	fail_unless(ret == -XMP_ERROR_INVALID, "load sample in invalid slot");

	/* try to load non-existant file */
	ret = xmp_smix_load_sample(opaque, 0, "doesnt.exist");
	fail_unless(ret == -XMP_ERROR_SYSTEM, "sample doesn't exist");
	fail_unless(errno == ENOENT, "errno");

	/* try to load sample with invalid format */
        ret = xmp_smix_load_sample(opaque, 1, "data/mod.loving_is_easy.pp");
	fail_unless(ret == -XMP_ERROR_FORMAT, "invalid format");

	/* try to load stereo sample */
        ret = xmp_smix_load_sample(opaque, 1, "data/send.wav");
	fail_unless(ret == -XMP_ERROR_FORMAT, "invalid format");

	ret = xmp_smix_load_sample(opaque, 0, "data/blip.wav");
	fail_unless(ret == 0, "load sample");

	xmp_smix_release_sample(opaque, 0);

	xmp_end_smix(opaque);
}
END_TEST

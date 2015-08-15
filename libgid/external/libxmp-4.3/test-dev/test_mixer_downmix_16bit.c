#include "test.h"
#include "../src/effects.h"

TEST(test_mixer_downmix_16bit)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct xmp_frame_info info;
	FILE *f;
	int i, j, val;

	f = fopen("data/downmix.data", "r");

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;

	xmp_load_module(opaque, "data/test.xm");

	new_event(ctx, 0, 0, 0, 48, 1, 0, 0x0f, 2, 0, 0);

	xmp_start_player(opaque, 22050, XMP_FORMAT_MONO);

	for (i = 0; i < 2; i++) {
		int16 *b;
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		b = info.buffer;
		for (j = 0; j < info.buffer_size / 2; j++) {
			fscanf(f, "%d", &val);
			fail_unless(b[j] == val, "downmix error");
		}
	}

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

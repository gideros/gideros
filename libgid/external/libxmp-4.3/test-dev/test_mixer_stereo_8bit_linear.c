#include "test.h"

TEST(test_mixer_stereo_8bit_linear)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct mixer_data *s;
	struct xmp_frame_info info;
	FILE *f;
	int i, j, k, val;

	f = fopen("data/mixer_8bit_linear.data", "r");

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	s = &ctx->s;

	xmp_load_module(opaque, "data/test.xm");

	for (i = 0; i < 5; i++) {
		new_event(ctx, 0, i, 0, 20 + i * 20, 1, 0, 0x0f, 2, 0, 0);
	}

	xmp_start_player(opaque, 8000, 0);
	xmp_set_player(opaque, XMP_PLAYER_INTERP, XMP_INTERP_LINEAR);

	for (i = 0; i < 10; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		for (k = j = 0; j < info.buffer_size / 4; j++) {
			fscanf(f, "%d", &val);
			fail_unless(s->buf32[k++] == val, "mixing error L");
			fail_unless(s->buf32[k++] == val, "mixing error R");
		}
	}

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

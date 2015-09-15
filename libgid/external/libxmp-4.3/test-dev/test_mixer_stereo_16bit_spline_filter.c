#include "test.h"
#include "../src/effects.h"

TEST(test_mixer_stereo_16bit_spline_filter)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct mixer_data *s;
	struct xmp_frame_info info;
	FILE *f;
	int i, j, k, val;

	f = fopen("data/mixer_16bit_spline_filter.data", "r");

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	s = &ctx->s;

	xmp_load_module(opaque, "data/test.it");

	new_event(ctx, 0, 0, 0, 30, 2, 0, 0x0f, 2, FX_FLT_CUTOFF, 50);
	new_event(ctx, 0, 1, 0, 30, 2, 0, 0x0f, 2, FX_FLT_CUTOFF, 120);

	xmp_start_player(opaque, 22050, 0);
	xmp_set_player(opaque, XMP_PLAYER_INTERP, XMP_INTERP_SPLINE);

	for (i = 0; i < 4; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		for (k = j = 0; j < info.buffer_size / 4; j++) {
			fscanf(f, "%d", &val);
			fail_unless(abs(s->buf32[k++] - val) <= 1, "mixing error L");
			fail_unless(abs(s->buf32[k++] - val) <= 1, "mixing error R");
		}
	}

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

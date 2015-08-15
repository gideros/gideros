#include "test.h"
#include "../src/loaders/loader.h"

TEST(test_effect_ef_invert_loop)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct mixer_data *s;
	struct module_data *m;
	struct xmp_frame_info info;
	HIO_HANDLE *h;
	FILE *f;
	int i, j, val;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	s = &ctx->s;
	m = &ctx->m;

	create_simple_module(ctx, 2, 2);
	set_quirk(ctx, QUIRK_PROTRACK, READ_EVENT_MOD);
	h = hio_open("data/sample-square-8bit.raw", "rb");
	fail_unless(h != NULL, "can't open sample file");
	
	m->mod.xxs[0].len = 40;
	m->mod.xxs[0].lps = 0;
	m->mod.xxs[0].lpe = 40;
	load_sample(m, h, 0, &m->mod.xxs[0], NULL);
	hio_close(h);

	new_event(ctx, 0, 0, 0, 49, 1, 0, 0x0e, 0xfe, 0x0f, 1);

	f = fopen("data/invloop.data", "r");

	xmp_start_player(opaque, 16000, XMP_FORMAT_MONO);
	xmp_set_player(opaque, XMP_PLAYER_INTERP, XMP_INTERP_NEAREST);

	for (i = 0; i < 6; i++) {
		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		for (j = 0; j < info.buffer_size / 2; j++) {
			fscanf(f, "%d", &val);
			fail_unless(s->buf32[j] == val, "invloop error");
		}
	}

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

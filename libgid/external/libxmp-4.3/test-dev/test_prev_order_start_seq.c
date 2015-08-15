#include "test.h"

TEST(test_prev_order_start_seq)
{
	xmp_context opaque;
	struct context_data *ctx;
	struct player_data *p;
	int i;

	opaque = xmp_create_context();
	ctx = (struct context_data *)opaque;
	p = &ctx->p;

 	create_simple_module(ctx, 2, 2);
	set_order(ctx, 0, 0);
	set_order(ctx, 1, 0xfe);
	set_order(ctx, 2, 0);
	set_order(ctx, 3, 0xff);
	set_order(ctx, 4, 1);
	prepare_scan(ctx);
	scan_sequences(ctx);

	xmp_start_player(opaque, 44100, 0);
	xmp_set_position(opaque, 4);
	xmp_play_frame(opaque);
	fail_unless(p->ord == 4, "didn't start at pattern 4");
	fail_unless(p->sequence == 1, "wrong sequence");

	for (i = 0; i < 30; i++) {
		xmp_play_frame(opaque);
	}

	xmp_prev_position(opaque);
	xmp_play_frame(opaque);
	fail_unless(p->ord == 4, "incorrect pattern");
	fail_unless(p->row == 0, "incorrect row");
	fail_unless(p->frame == 0, "incorrect frame");

}
END_TEST

#include "test.h"

TEST(test_api_restart_module)
{
	xmp_context ctx;
	struct xmp_frame_info info;
	int ret;
	int i;

	ctx = xmp_create_context();
	xmp_load_module(ctx, "data/ode2ptk.mod");
	xmp_start_player(ctx, 8000, 0);

	for (i = 0; i < 100; i++) {
		ret = xmp_play_frame(ctx);
		fail_unless(ret == 0, "play frame error");
	}

	xmp_restart_module(ctx);

	ret = xmp_play_frame(ctx);
	fail_unless(ret == 0, "play frame error");

	xmp_get_frame_info(ctx, &info);
	fail_unless(info.pos == 0, "module restart error");
	

}
END_TEST

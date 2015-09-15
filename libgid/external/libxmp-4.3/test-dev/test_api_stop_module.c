#include "test.h"

TEST(test_api_stop_module)
{
	xmp_context ctx;
	int ret;
	int i;

	ctx = xmp_create_context();
	xmp_load_module(ctx, "data/ode2ptk.mod");
	xmp_start_player(ctx, 8000, 0);

	for (i = 0; i < 100; i++) {
		ret = xmp_play_frame(ctx);
		fail_unless(ret == 0, "play frame error");
	}

	xmp_stop_module(ctx);

	ret = xmp_play_frame(ctx);
	fail_unless(ret == -XMP_END, "module stop error");
	

}
END_TEST

#include "test.h"

TEST(test_player_scan)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	int ret;

	opaque = xmp_create_context();
	fail_unless(opaque != NULL, "can't create context");

	ret = xmp_load_module(opaque, "data/ode2ptk.mod");
	fail_unless(ret == 0, "can't load module");

	xmp_start_player(opaque, 44100, 0);
	xmp_get_frame_info(opaque, &info);
	fail_unless(info.total_time == 85472, "incorrect total time");
	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

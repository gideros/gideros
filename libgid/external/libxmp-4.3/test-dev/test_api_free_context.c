#include "test.h"

TEST(test_api_free_context)
{
	xmp_context ctx;
	int state, ret;

	ctx = xmp_create_context();
	fail_unless(ctx != 0, "returned NULL");

	state = xmp_get_player(ctx, XMP_PLAYER_STATE);
	fail_unless(state == XMP_STATE_UNLOADED, "state error");

	/* load module */
	ret = xmp_load_module(ctx, "data/test.xm");
	fail_unless(ret == 0, "load file");

	state = xmp_get_player(ctx, XMP_PLAYER_STATE);
	fail_unless(state == XMP_STATE_LOADED, "state error");

	/* start playing */
        ret = xmp_start_player(ctx, 44100, 0);
        fail_unless(ret == 0, "min sample rate failed");

        state = xmp_get_player(ctx, XMP_PLAYER_STATE);
        fail_unless(state == XMP_STATE_PLAYING, "state error");

	/* Free context while playing */
	xmp_free_context(ctx);
}
END_TEST

#include "test.h"

TEST(test_api_create_module)
{
	xmp_context ctx;
	int state, ret;
	struct xmp_module_info mi;
	struct xmp_module *mod;

	ctx = xmp_create_context();

	state = xmp_get_player(ctx, XMP_PLAYER_STATE);
	fail_unless(state == XMP_STATE_UNLOADED, "state error");

	ret = xmp_create_module(ctx, 4);
	fail_unless(ret == 0, "create module");

	state = xmp_get_player(ctx, XMP_PLAYER_STATE);
	fail_unless(state == XMP_STATE_LOADED, "state error");

	xmp_get_module_info(ctx, &mi);
	mod = mi.mod;
	
	fail_unless(mod->chn == 4, "number of channels");
	fail_unless(mod->len == 1, "module length");
	fail_unless(mod->pat == 1, "number of patterns");
	fail_unless(mod->trk == 4, "number of tracks");

	/* unload */
	xmp_release_module(ctx);

	state = xmp_get_player(ctx, XMP_PLAYER_STATE);
	fail_unless(state == XMP_STATE_UNLOADED, "state error");
}
END_TEST

#include <stdlib.h>
#include "test.h"


TEST(test_api_load_module_from_file)
{
	xmp_context ctx;
	FILE *f;
	int state, ret;

	ctx = xmp_create_context();

	f = fopen("data/test.it", "rb");
	fail_unless(f != NULL, "open file");

	ret = xmp_load_module_from_file(ctx, f, 0);
	fclose(f);
	fail_unless(ret == 0, "load file");

	state = xmp_get_player(ctx, XMP_PLAYER_STATE);
	fail_unless(state == XMP_STATE_LOADED, "state error");

	/* unload */
	xmp_release_module(ctx);

	state = xmp_get_player(ctx, XMP_PLAYER_STATE);
	fail_unless(state == XMP_STATE_UNLOADED, "state error");
}
END_TEST

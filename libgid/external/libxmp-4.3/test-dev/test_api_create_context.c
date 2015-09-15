#include "test.h"

TEST(test_api_create_context)
{
	xmp_context ctx;
	int state;

	ctx = xmp_create_context();
	fail_unless(ctx != 0, "returned NULL");

	state = xmp_get_player(ctx, XMP_PLAYER_STATE);
	fail_unless(state == XMP_STATE_UNLOADED, "state error");
}
END_TEST

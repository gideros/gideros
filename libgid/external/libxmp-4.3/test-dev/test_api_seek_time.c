#include "test.h"

int pos[] = {
	0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 
	2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 
	5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9, 
	10, 10, 10, 11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 
	13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 
	16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 
	17, 17, 17, 17, 17, 17, 17, 17, 17, 17
};


TEST(test_api_seek_time)
{
	xmp_context ctx;
	int ret;
	int i;

	ctx = xmp_create_context();
	xmp_load_module(ctx, "data/ode2ptk.mod");
	xmp_start_player(ctx, 8000, 0);

	for (i = 0; i < 100; i++) {
		ret = xmp_seek_time(ctx, i * 1000);
		fail_unless(ret == pos[i], "seek error");
	}

}
END_TEST

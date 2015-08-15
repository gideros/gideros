#include "test.h"

TEST(test_synth_adlib)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	int i, j, k, val, ret;
	FILE *f;

	f = fopen("data/adlib.data", "r");

	opaque = xmp_create_context();
	fail_unless(opaque != NULL, "can't create context");

	ret = xmp_load_module(opaque, "data/adlibsp.rad.gz");
	fail_unless(ret == 0, "can't load module");

	xmp_start_player(opaque, 44100, 0);

	for (j = 0; j < 2; j++) {
		int16 *b;

		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		b = info.buffer;

		for (k = i = 0; i < info.buffer_size / 4; i++) {
			fscanf(f, "%d", &val);
			fail_unless(b[k++] == val, "Adlib error L");
			fail_unless(b[k++] == val, "Adlib error R");
		}
	}
	
	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

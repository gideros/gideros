#include "test.h"

TEST(test_synth_spectrum)
{
	xmp_context opaque;
	struct xmp_frame_info info;
	int i, j, val, ret;
	FILE *f;

	f = fopen("data/spectrum.data", "r");

	opaque = xmp_create_context();
	fail_unless(opaque != NULL, "can't create context");

	ret = xmp_load_module(opaque, "data/again.stc");
	fail_unless(ret == 0, "can't load module");

	xmp_start_player(opaque, 22050, 0);
	xmp_set_position(opaque, 2);

	for (j = 0; j < 2; j++) {
		int16 *b;

		xmp_play_frame(opaque);
		xmp_get_frame_info(opaque, &info);
		b = info.buffer;

		for (i = 0; i < info.buffer_size / 2; i++) {
			fscanf(f, "%d", &val);
			fail_unless(b[i] == val, "synth error");
		}
	}
	
	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

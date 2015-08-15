#include "test.h"
#include "../src/loaders/loader.h"

static int vals[] = { 11, 117, 313, 701, 1111, 3999, 7071, 10037, -1 };
static char buffer[20000];

#define REFBUF_SIZE 72000

TEST(test_api_play_buffer)
{
	xmp_context opaque;
	FILE *f;
	int i, ret, cmp, size, buffer_size;
	char *ref_buffer;

	f = fopen("data/pcm_buffer.raw", "rb");
	ref_buffer = calloc(1, REFBUF_SIZE);
	fail_unless(ref_buffer != NULL, "buffer allocation error");

	fread(ref_buffer, 1, REFBUF_SIZE, f);
	if (is_big_endian()) {
		convert_endian((unsigned char *)ref_buffer, REFBUF_SIZE / 2);
	}

	opaque = xmp_create_context();

	ret = xmp_load_module(opaque, "data/storlek_03.it");
	fail_unless(ret == 0, "module load error");

	xmp_start_player(opaque, 8000, XMP_FORMAT_MONO);
	xmp_set_player(opaque, XMP_PLAYER_INTERP, XMP_INTERP_LINEAR);

#if 1
	for (i = 0; vals[i] > 0; i++) {
		xmp_restart_module(opaque);
		size = 0;
		buffer_size = vals[i];

		xmp_play_buffer(opaque, NULL, 0, 0);

		while ((ret = xmp_play_buffer(opaque, buffer, buffer_size, 1)) == 0) {
			cmp = memcmp(buffer, ref_buffer + size, buffer_size);
			fail_unless(cmp == 0, "buffer comparison failed");

			size += buffer_size;
			if ((size + buffer_size) >= REFBUF_SIZE)
				break;
		}

		/* check end of module */
		fail_unless(ret == -1, "end of module");
	}

	fail_unless(vals[i] == -1, "didn't test all buffer sizes");

#else

	struct xmp_frame_info fi;

	for (i = 0; i < 188; i++) {
		if (xmp_play_frame(opaque) < 0)
			break;
		xmp_get_frame_info(opaque, &fi);
		if (fi.loop_count > 0)
			break;
		fwrite(fi.buffer, 1, fi.buffer_size, f);
	}
#endif

	xmp_end_player(opaque);
	xmp_release_module(opaque);
	xmp_free_context(opaque);
	fclose(f);
}
END_TEST

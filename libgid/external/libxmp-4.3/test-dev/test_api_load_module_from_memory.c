#include <stdio.h>
#include "test.h"

#define BUFFER_SIZE 256000

static unsigned char *buffer;


TEST(test_api_load_module_from_memory)
{
	xmp_context ctx;
	struct xmp_frame_info fi;
	int ret, size;
	FILE *f;

	buffer = malloc(BUFFER_SIZE);
	fail_unless(buffer != NULL, "buffer allocation");

	ctx = xmp_create_context();
	f = fopen("data/test.xm", "rb");
	fail_unless(f != NULL, "can't open module");
	size = fread(buffer, 1, BUFFER_SIZE, f);
	fclose(f);

	/* valid file */
	ret = xmp_load_module_from_memory(ctx, buffer, size);
	fail_unless(ret == 0, "load file");

	xmp_get_frame_info(ctx, &fi);
	fail_unless(fi.total_time == 15360, "module duration");

	f = fopen("data/test.it", "rb");
	fail_unless(f != NULL, "can't open module");
	size = fread(buffer, 1, BUFFER_SIZE, f);
	fclose(f);

	/* and reload without releasing */
	ret = xmp_load_module_from_memory(ctx, buffer, size);
	fail_unless(ret == 0, "load file");

	xmp_get_frame_info(ctx, &fi);
	fail_unless(fi.total_time == 7680, "module duration");


	/* reported crashing in 4.2.0 by Andreas Argirakis */
	xmp_release_module(ctx);
	f = fopen("data/m/reborning.mod", "rb");
	fail_unless(f != NULL, "can't open module");
	size = fread(buffer, 1, BUFFER_SIZE, f);
	fclose(f);

	ret = xmp_load_module_from_memory(ctx, buffer, size);
	fail_unless(ret == 0, "load file");

	xmp_get_frame_info(ctx, &fi);
	fail_unless(fi.total_time == 107520, "module duration");


	/* load through a prowizard converter */
	xmp_release_module(ctx);
	f = fopen("data/m/mod.sad-song", "rb");
	fail_unless(f != NULL, "can't open module");
	size = fread(buffer, 1, BUFFER_SIZE, f);
	fclose(f);

	ret = xmp_load_module_from_memory(ctx, buffer, size);
	fail_unless(ret == 0, "load file");

	xmp_get_frame_info(ctx, &fi);
	fail_unless(fi.total_time == 235520, "module duration");

	free(buffer);
}
END_TEST

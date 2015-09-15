#include "test.h"


TEST(test_depack_vorbis_8bit)
{
	FILE *f;
	struct stat st;
	int i, ret;
	int8 *buf, *pcm8;
	xmp_context c;
	struct xmp_module_info info;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");

	ret = xmp_load_module(c, "data/jerry-boleti.oxm");
	fail_unless(ret == 0, "can't load module");

	xmp_start_player(c, 44100, 0);
	xmp_get_module_info(c, &info);

	stat("data/sample4.raw", &st);
	f = fopen("data/sample4.raw", "rb");
	fail_unless(f != NULL, "can't open raw data file");

	buf = malloc(st.st_size);
	fail_unless(buf != NULL, "can't alloc raw buffer");
	fread(buf, 1, st.st_size, f);
	fclose(f);

	pcm8 = (int8 *)info.mod->xxs[4].data;

	for (i = 0; i < 5492; i++) {
		if (pcm8[i] != buf[i])
			printf("%d %d\n", pcm8[i], buf[i]);
			fail_unless(abs(pcm8[i] - buf[i]) <= 1, "data error");
	}
}
END_TEST

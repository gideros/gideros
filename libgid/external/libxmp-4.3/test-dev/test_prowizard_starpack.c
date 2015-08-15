#include "test.h"


TEST(test_prowizard_starpack)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/CAVE-EchoBBS-1.StarTrekkerPack");
	fail_unless(ret == 0, "can't load module");

	xmp_start_player(c, 44100, 0);
	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "f8a1bc8a19e7dfc446664494cd22788a");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

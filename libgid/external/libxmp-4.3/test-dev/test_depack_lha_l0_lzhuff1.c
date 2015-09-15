#include "test.h"


TEST(test_depack_lha_l0_lzhuff1)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/l0_lzhuff1");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "dde7301ad7957daeede383fd561e12df");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

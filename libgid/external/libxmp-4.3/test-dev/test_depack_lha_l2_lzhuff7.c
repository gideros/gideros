#include "test.h"


TEST(test_depack_lha_l2_lzhuff7)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/l2_lzhuff7");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "d1b4a2c15dcd1730244e2334eae30876");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

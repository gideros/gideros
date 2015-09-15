#include "test.h"


TEST(test_depack_lha_l0_lzhuff5)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/l0_lzhuff5");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "d62117b9d24b152b225bdb7be24d5c5c");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

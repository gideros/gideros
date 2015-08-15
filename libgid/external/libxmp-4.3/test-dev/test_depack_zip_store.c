#include "test.h"


TEST(test_depack_zip_store)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/zipdata3");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "d5d4b02731591ecc350f6e18d8e61c6a");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

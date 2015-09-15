#include "test.h"


TEST(test_depack_arc_method8)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/arc-method8-rle");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "64d67d1d5d123c6542a8099255ad8ca2");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

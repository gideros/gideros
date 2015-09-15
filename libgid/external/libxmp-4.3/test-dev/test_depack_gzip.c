#include "test.h"


TEST(test_depack_gzip)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/gzipdata");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "0350baf25b96d6d125f537c63f03e3db");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

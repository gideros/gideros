#include "test.h"


TEST(test_depack_zoo)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/zoodata.zoo");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "a3c22f92e1ec5d7d324e975364a775e8");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

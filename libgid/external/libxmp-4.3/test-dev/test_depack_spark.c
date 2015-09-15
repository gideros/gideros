#include "test.h"


TEST(test_depack_spark)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/038984");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "1aecc3cbfdae12a76000cd048ba8fcb3");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

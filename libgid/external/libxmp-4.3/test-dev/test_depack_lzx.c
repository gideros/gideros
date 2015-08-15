#include "test.h"


TEST(test_depack_lzx)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/lzxdata");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "6e4226be5a72fe3770550ced7a2022de");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

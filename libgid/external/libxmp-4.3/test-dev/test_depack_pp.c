#include "test.h"


TEST(test_depack_pp)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/mod.loving_is_easy.pp");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "80ba11ca20f7ffef184a58c1fc619c18");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

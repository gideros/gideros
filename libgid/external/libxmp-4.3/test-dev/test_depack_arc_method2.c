#include "test.h"


TEST(test_depack_arc_method2)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/arc-method2");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "b868aef5b8843c0754284561cda5aba9");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

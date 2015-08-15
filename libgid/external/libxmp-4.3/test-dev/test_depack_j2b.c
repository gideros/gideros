#include "test.h"


TEST(test_depack_j2b)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/j2b_muse_data");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "166d2a8f5e4fbb466fe15d820ca59265");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

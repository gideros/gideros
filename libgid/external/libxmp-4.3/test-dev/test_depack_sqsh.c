#include "test.h"


TEST(test_depack_sqsh)
{
	xmp_context c;
	struct xmp_module_info info;
	int ret;

	c = xmp_create_context();
	fail_unless(c != NULL, "can't create context");
	ret = xmp_load_module(c, "data/PRU2.PDX-Perihelion");
	fail_unless(ret == 0, "can't load module");

	xmp_get_module_info(c, &info);

	ret = compare_md5(info.md5, "70e931a4c9835cd60493ae3d22b0cea621");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

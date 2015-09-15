#include "test.h"

TEST(test_loader_liq_no)
{
	xmp_context opaque;
	struct xmp_module_info info;
	FILE *f;
	int ret;

	f = fopen("data/format_liq_no.data", "r");

	opaque = xmp_create_context();
	ret = xmp_load_module(opaque, "data/m/time after time.liq");
	fail_unless(ret == 0, "module load");

	xmp_get_module_info(opaque, &info);

	ret = compare_module(info.mod, f);
	fail_unless(ret == 0, "format not correctly loaded");

	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

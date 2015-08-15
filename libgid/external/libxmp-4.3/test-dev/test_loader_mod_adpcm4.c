#include "test.h"

TEST(test_loader_mod_adpcm4)
{
	xmp_context opaque;
	struct xmp_module_info info;
	FILE *f;
	int ret;

	f = fopen("data/format_mod_adpcm4.data", "r");

	opaque = xmp_create_context();
	ret = xmp_load_module(opaque, "data/m/fairli.mod");
	fail_unless(ret == 0, "module load");

	xmp_get_module_info(opaque, &info);

	ret = compare_module(info.mod, f);
	fail_unless(ret == 0, "format not correctly loaded");

	xmp_release_module(opaque);
	xmp_free_context(opaque);
}
END_TEST

#include "test.h"
#include "../src/loaders/loader.h"

struct xmp_sample xxs;

TEST(test_sample_load_delta)
{
	int8 buffer0[10] = { 0, 1, 2, 3,  4,  5,  6, -7,  8, -29 };
	int8 conv_r0[10] = { 0, 1, 3, 6, 10, 15, 21, 14, 22,  -7 };
	/* 16-bit input buffer is little-endian */
	uint8  buffer1[20] = { 0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0,
			       6, 0, 0xf9, 0xff, 8, 0, 0xe3, 0xff };
	/* 16-bit output buffer is native-endian */
	uint16 conv_r1[10] = { 0, 1, 3, 6, 10, 15, 21, 14, 22, 65529 };
	struct module_data m;

	memset(&m, 0, sizeof(struct module_data));

	xxs.len = 10;
	load_sample(&m, NULL, SAMPLE_FLAG_NOLOAD | SAMPLE_FLAG_DIFF, &xxs, buffer0);
	fail_unless(memcmp(xxs.data, conv_r0, 10) == 0,
				"Invalid 8-bit conversion");

	xxs.flg = XMP_SAMPLE_16BIT;
	load_sample(&m, NULL, SAMPLE_FLAG_NOLOAD | SAMPLE_FLAG_DIFF, &xxs, buffer1);
	fail_unless(memcmp(xxs.data, conv_r1, 20) == 0,
				"Invalid 16-bit conversion");
}
END_TEST

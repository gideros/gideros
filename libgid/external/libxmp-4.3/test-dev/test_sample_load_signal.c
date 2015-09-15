#include "test.h"
#include "../src/loaders/loader.h"

struct xmp_sample xxs;

TEST(test_sample_load_signal)
{
	int8  buffer0[10] = { 0, 1, 2, 3,  4,  5,  6, -7,  8, -29 };
	uint8 conv_r0[10] = {
		128, 129, 130, 131, 132, 133, 134, 121, 136, 99
	};

	/* 16-bit input buffer is little-endian */
	uint8  buffer1[20] = {
		0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 5, 0,
		6, 0, 0xf9, 0xff, 8, 0, 0xe3, 0xff
	};
	/* 16-bit input buffer is native-endian */
	uint16 conv_r1[10] = {
		32768, 32769, 32770, 32771, 32772, 
		32773, 32774, 32761, 32776, 32739
	};
	struct module_data m;

	memset(&m, 0, sizeof (struct module_data));

	xxs.len = 10;
	load_sample(&m, NULL, SAMPLE_FLAG_NOLOAD | SAMPLE_FLAG_UNS, &xxs, buffer0);
	fail_unless(memcmp(xxs.data, conv_r0, 10) == 0,
				"Invalid 8-bit conversion");

	xxs.flg = XMP_SAMPLE_16BIT;
	load_sample(&m, NULL, SAMPLE_FLAG_NOLOAD | SAMPLE_FLAG_UNS, &xxs, buffer1);
	fail_unless(memcmp(xxs.data, conv_r1, 20) == 0,
				"Invalid 16-bit conversion");
}
END_TEST

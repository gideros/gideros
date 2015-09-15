#include "test.h"

TEST(test_read_file_32bit_little_endian)
{
	FILE *f;
	int x;

	f = fopen("data/test.mmcmp", "rb");
	fail_unless(f != NULL, "can't open data file");

	x = read32l(f);
	fail_unless(x == 0x4352697a, "read error");
}
END_TEST

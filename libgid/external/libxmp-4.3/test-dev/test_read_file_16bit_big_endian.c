#include "test.h"

TEST(test_read_file_16bit_big_endian)
{
	FILE *f;
	int x;

	f = fopen("data/test.mmcmp", "rb");
	fail_unless(f != NULL, "can't open data file");

	x = read16b(f);
	fail_unless(x == 0x00007a69, "read error");
}
END_TEST

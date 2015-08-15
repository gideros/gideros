#include "test.h"

TEST(test_read_file_24bit_big_endian)
{
	FILE *f;
	int x;

	f = fopen("data/test.mmcmp", "rb");
	fail_unless(f != NULL, "can't open data file");

	x = read24b(f);
	fail_unless(x == 0x007a6952, "read error");
}
END_TEST

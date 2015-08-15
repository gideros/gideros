#include "test.h"

TEST(test_read_file_8bit)
{
	FILE *f;
	int x;

	f = fopen("data/test.mmcmp", "rb");
	fail_unless(f != NULL, "can't open data file");

	x = read8(f);
	fail_unless(x == 0x0000007a, "read error");
}
END_TEST

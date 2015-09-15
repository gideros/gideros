#include "test.h"

TEST(test_write_file_32bit_little_endian)
{
	FILE *f;
	int x;

	f = fopen("write_test", "wb");
	fail_unless(f != NULL, "can't open data file");

	write32l(f, 0x12345678);
	fclose(f);

	f = fopen("write_test", "rb");
	x = read32l(f);
	fail_unless(x == 0x12345678, "read error");
}
END_TEST

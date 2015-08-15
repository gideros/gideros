#include "test.h"

TEST(test_write_file_16bit_big_endian)
{
	FILE *f;
	int x;

	f = fopen("write_test", "wb");
	fail_unless(f != NULL, "can't open data file");

	write16b(f, 0x1234);
	fclose(f);

	f = fopen("write_test", "rb");
	x = read16b(f);
	fail_unless(x == 0x1234, "read error");
}
END_TEST

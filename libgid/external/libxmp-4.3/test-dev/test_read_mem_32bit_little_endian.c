#include "test.h"

TEST(test_read_mem_32bit_little_endian)
{
	int x;
	uint8 mem[10] = { 1, 2, 3, 4 };

	x = readmem32l(mem);
	fail_unless(x == 0x04030201, "read error");
}
END_TEST

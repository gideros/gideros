#include "test.h"

TEST(test_read_mem_32bit_big_endian)
{
	int x;
	uint8 mem[10] = { 1, 2, 3, 4 };

	x = readmem32b(mem);
	fail_unless(x == 0x01020304, "read error");
}
END_TEST

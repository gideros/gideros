#include "test.h"

TEST(test_read_mem_16bit_big_endian)
{
	int x;
	uint8 mem[10] = { 1, 2, 3, 4 };

	x = readmem16b(mem);
	fail_unless(x == 0x00000102, "read error");
}
END_TEST

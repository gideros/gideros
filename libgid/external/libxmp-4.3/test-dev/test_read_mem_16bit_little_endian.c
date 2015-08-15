#include "test.h"

TEST(test_read_mem_16bit_little_endian)
{
	int x;
	uint8 mem[10] = { 1, 2, 3, 4 };

	x = readmem16l(mem);
	fail_unless(x == 0x00000201, "read error");
}
END_TEST

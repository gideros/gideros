#include <sys/types.h>
#include <sys/stat.h>
#include "test.h"
#include "../src/hio.h"

TEST(test_read_mem_hio)
{
	uint8 mem[100], mem2[100];
	int i;
	int x;
	HIO_HANDLE *h;
	struct stat st;

	for (i = 0; i < 100; i++)
		mem[i] = i;

	h = hio_open_mem(mem, 100);
	fail_unless(h != NULL, "hio_open");

	x = hio_size(h);
	fail_unless(x == 100, "hio_size");

	x = hio_read8(h);
	fail_unless(x == 0x00, "hio_read8");

	x = hio_read8s(h);
	fail_unless(x == 0x01, "hio_read8s");

	x = hio_read16l(h);
	fail_unless(x == 0x0302, "hio_read16l");

	x = hio_read16b(h);
	fail_unless(x == 0x0405, "hio_read16b");

	x = hio_read24l(h);
	fail_unless(x == 0x080706, "hio_read24l");

	x = hio_read24b(h);
	fail_unless(x == 0x090a0b, "hio_read24b");

	x = hio_read32l(h);
	fail_unless(x == 0x0f0e0d0c, "hio_read32l");

	x = hio_read32b(h);
	fail_unless(x == 0x10111213, "hio_read32b");

	x = hio_tell(h);
	fail_unless(x == 0x14, "hio_fseek");

	x = hio_seek(h, 2, SEEK_SET);
	fail_unless(x == 0, "hio_fseek SEEK_SET");

	x = hio_read32b(h);
	fail_unless(x == 0x02030405, "hio_read32b");

	x = hio_seek(h, 3, SEEK_CUR);
	fail_unless(x == 0, "hio_fseek SEEK_CUR");

	x = hio_read(mem2, 1, 50, h);
	for (i = 0; i < 50; i++)
		fail_unless(mem2[i] == i + 9, "hio_read");

	x = hio_seek(h, 0, SEEK_END);
	fail_unless(x == 0, "hio_fseek SEEK_END");

	x = hio_read8(h);
	x = hio_eof(h);
	fail_unless(x != 0, "read8 eof");

	x = hio_read8s(h);
	x = hio_eof(h);
	fail_unless(x != 0, "read8s eof");

	x = hio_read16l(h);
	x = hio_eof(h);
	fail_unless(x != 0, "read16l eof");

	x = hio_read16b(h);
	x = hio_eof(h);
	fail_unless(x != 0, "read16b eof");

	x = hio_read24l(h);
	x = hio_eof(h);
	fail_unless(x != 0, "read24l eof");

	x = hio_read24b(h);
	x = hio_eof(h);
	fail_unless(x != 0, "read24b eof");

	x = hio_read32l(h);
	x = hio_eof(h);
	fail_unless(x != 0, "read32l eof");

	x = hio_read32b(h);
	x = hio_eof(h);
	fail_unless(x != 0, "read32b eof");

	x = hio_seek(h, 20, SEEK_CUR);
	fail_unless(x == -1, "hio_seek");

	x = hio_close(h);
	fail_unless(x == 0, "hio_close");
}
END_TEST

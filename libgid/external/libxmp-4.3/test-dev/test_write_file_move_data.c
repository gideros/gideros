#include "test.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

TEST(test_write_file_move_data)
{
	FILE *f1, *f2;
	uint8 b1[4000], b2[4000];
	struct stat st;

	f1 = fopen("data/bzip2data", "rb");
	fail_unless(f1 != NULL, "can't open source file");
	f2 = fopen("write_test", "wb");
	fail_unless(f1 != NULL, "can't open destination file");

	move_data(f2, f1, 4000);

	fclose(f1);
	fclose(f2);

	stat("write_test", &st);
	fail_unless(st.st_size == 4000, "wrong size");

	f1 = fopen("data/bzip2data", "rb");
	f2 = fopen("write_test", "rb");
	fread(b1, 1, 4000, f1);
	fread(b2, 1, 4000, f2);

	fail_unless(memcmp(b1, b2, 4000) == 0, "read error");
}
END_TEST

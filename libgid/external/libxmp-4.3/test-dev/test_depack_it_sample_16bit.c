#include "test.h"
#include "../src/loaders/loader.h"

int itsex_decompress16(HIO_HANDLE *module, void *dst, int len, char it215);


TEST(test_depack_it_sample_16bit)
{
	HIO_HANDLE *f;
	FILE *fo;
	int ret;
	char dest[10000];

	f = hio_open("data/it-sample-16bit.raw", "rb");
	fail_unless(f != NULL, "can't open data file");

	fo = fopen(TMP_FILE, "wb");
	fail_unless(fo != NULL, "can't open output file");

	ret = itsex_decompress16(f, dest, 4646, 0);
	fail_unless(ret == 0, "decompression fail");

	if (is_big_endian()) {
		convert_endian((unsigned char *)dest, 4646);
	}

	fwrite(dest, 1, 9292, fo);

	fclose(fo);
	hio_close(f);

	ret = check_md5(TMP_FILE, "1e2395653f9bd7838006572d8fcdb646");
	fail_unless(ret == 0, "MD5 error");
}
END_TEST

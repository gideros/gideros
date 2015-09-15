#include "test.h"
#include "../src/loaders/loader.h"

#define SET(x,y,z,w) do { \
  s.len = (x); s.lps = (y); s.lpe = (z); s.flg = (w); s.data = NULL; \
} while (0)


TEST(test_sample_load_16bit)
{
	struct xmp_sample s;
	HIO_HANDLE *f;
	short buffer[202];
	int i;
	struct module_data m;

	memset(&m, 0, sizeof(struct module_data));

	f = hio_open("data/sample-16bit.raw", "rb");
	fail_unless(f != NULL, "can't open sample file");

	/* read little-endian sample to native-endian buffer */
	for (i = 0; i < 101; i++) {
		buffer[i] = hio_read16l(f);
	}
	for (i = 0; i < 101; i++) {
		buffer[101 + i] = buffer[101 - i - 1];
	}

	/* load zero-length sample */
	SET(0, 0, 101, XMP_SAMPLE_16BIT | XMP_SAMPLE_LOOP);
	load_sample(&m, NULL, 0, &s, NULL);

	/* load sample with invalid loop */
	SET(101, 150, 180, XMP_SAMPLE_16BIT | XMP_SAMPLE_LOOP | XMP_SAMPLE_LOOP_BIDIR);
	hio_seek(f, 0, SEEK_SET);
	load_sample(&m, f, 0, &s, NULL);
	fail_unless(s.data != NULL, "didn't allocate sample data");
	fail_unless(s.lps == 0, "didn't fix invalid loop start");
	fail_unless(s.lpe == 0, "didn't fix invalid loop end");
	fail_unless(s.flg == XMP_SAMPLE_16BIT, "didn't reset loop flags");

	/* load sample with invalid loop */
	SET(101, 50, 40, XMP_SAMPLE_16BIT | XMP_SAMPLE_LOOP | XMP_SAMPLE_LOOP_BIDIR);
	hio_seek(f, 0, SEEK_SET);
	load_sample(&m, f, 0, &s, NULL);
	fail_unless(s.data != NULL, "didn't allocate sample data");
	fail_unless(s.lps == 0, "didn't fix invalid loop start");
	fail_unless(s.lpe == 0, "didn't fix invalid loop end");
	fail_unless(s.flg == XMP_SAMPLE_16BIT, "didn't reset loop flags");

	/* load sample from file */
	SET(101, 0, 102, XMP_SAMPLE_16BIT);
	hio_seek(f, 0, SEEK_SET);
	load_sample(&m, f, 0, &s, NULL);
	fail_unless(s.data != NULL, "didn't allocate sample data");
	fail_unless(s.lpe == 101, "didn't fix invalid loop end");
	fail_unless(memcmp(s.data, buffer, 202) == 0, "sample data error");
	fail_unless(s.data[202] == s.data[200], "sample adjust error");
	fail_unless(s.data[203] == s.data[201], "sample adjust error");
	fail_unless(s.data[204] == s.data[202], "sample adjust error");
	fail_unless(s.data[205] == s.data[203], "sample adjust error");

	/* load sample from file w/ loop */
	SET(101, 20, 80, XMP_SAMPLE_16BIT | XMP_SAMPLE_LOOP);
	hio_seek(f, 0, SEEK_SET);
	load_sample(&m, f, 0, &s, NULL);
	fail_unless(s.data != NULL, "didn't allocate sample data");
	fail_unless(s.data[160] == s.data[40], "sample adjust error");
	fail_unless(s.data[161] == s.data[41], "sample adjust error");
	fail_unless(s.data[162] == s.data[42], "sample adjust error");
	fail_unless(s.data[163] == s.data[43], "sample adjust error");

	/* load sample from w/ bidirectional loop */
	SET(101, 0, 102, XMP_SAMPLE_16BIT | XMP_SAMPLE_LOOP | XMP_SAMPLE_LOOP_BIDIR);
	hio_seek(f, 0, SEEK_SET);
	load_sample(&m, f, 0, &s, NULL);
	fail_unless(s.data != NULL, "didn't allocate sample data");
	fail_unless(s.lpe == 101, "didn't fix invalid loop end");
	fail_unless(memcmp(s.data, buffer, 404) == 0, "sample unroll error");
	fail_unless(s.data[404] == s.data[0], "sample adjust error");
	fail_unless(s.data[405] == s.data[1], "sample adjust error");
	fail_unless(s.data[406] == s.data[2], "sample adjust error");
	fail_unless(s.data[407] == s.data[3], "sample adjust error");

}
END_TEST

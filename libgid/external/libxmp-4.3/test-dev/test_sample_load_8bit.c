#include "test.h"
#include "../src/loaders/loader.h"

#define SET(x,y,z,w) do { \
  s.len = (x); s.lps = (y); s.lpe = (z); s.flg = (w); s.data = NULL; \
} while (0)


TEST(test_sample_load_8bit)
{
	struct xmp_sample s;
	HIO_HANDLE *f;
	char buffer[202];
	int i;
	struct module_data m;

	memset(&m, 0, sizeof (struct module_data));

	f = hio_open("data/sample-8bit.raw", "rb");
	fail_unless(f != NULL, "can't open sample file");
	hio_read(buffer, 1, 101, f);
	for (i = 0; i < 101; i++) {
		buffer[101 + i] = buffer[101 - i - 1];
	}


	/* load zero-length sample */
	SET(0, 0, 101, XMP_SAMPLE_LOOP);
	load_sample(&m, NULL, 0, &s, NULL);

	/* load sample with invalid loop */
	SET(101, 150, 180, XMP_SAMPLE_LOOP | XMP_SAMPLE_LOOP_BIDIR);
	hio_seek(f, 0, SEEK_SET);
	load_sample(&m, f, 0, &s, NULL);
	fail_unless(s.data != NULL, "didn't allocate sample data");
	fail_unless(s.lps == 0, "didn't fix invalid loop start");
	fail_unless(s.lpe == 0, "didn't fix invalid loop end");
	fail_unless(s.flg == 0, "didn't reset loop flags");

	/* load sample with invalid loop */
	SET(101, 50, 40, XMP_SAMPLE_LOOP | XMP_SAMPLE_LOOP_BIDIR);
	hio_seek(f, 0, SEEK_SET);
	load_sample(&m, f, 0, &s, NULL);
	fail_unless(s.data != NULL, "didn't allocate sample data");
	fail_unless(s.lps == 0, "didn't fix invalid loop start");
	fail_unless(s.lpe == 0, "didn't fix invalid loop end");
	fail_unless(s.flg == 0, "didn't reset loop flags");

	/* load sample from file */
	SET(101, 0, 102, 0);
	hio_seek(f, 0, SEEK_SET);
	load_sample(&m, f, 0, &s, NULL);
	fail_unless(s.data != NULL, "didn't allocate sample data");
	fail_unless(s.lpe == 101, "didn't fix invalid loop end");
	fail_unless(memcmp(s.data, buffer, 101) == 0, "sample data error");
	fail_unless(s.data[101] == s.data[100], "sample adjust error");
	fail_unless(s.data[102] == s.data[101], "sample adjust error");

	/* load sample from file w/ loop */
	SET(101, 20, 80, XMP_SAMPLE_LOOP);
	hio_seek(f, 0, SEEK_SET);
	load_sample(&m, f, 0, &s, NULL);
	fail_unless(s.data != NULL, "didn't allocate sample data");
	fail_unless(s.data[80] == s.data[20], "sample adjust error");
	fail_unless(s.data[81] == s.data[21], "sample adjust error");

	/* load sample from w/ bidirectional loop */
	SET(101, 0, 102, XMP_SAMPLE_LOOP | XMP_SAMPLE_LOOP_BIDIR);
	hio_seek(f, 0, SEEK_SET);
	load_sample(&m, f, 0, &s, NULL);
	fail_unless(s.data != NULL, "didn't allocate sample data");
	fail_unless(s.lpe == 101, "didn't fix invalid loop end");
	fail_unless(memcmp(s.data, buffer, 202) == 0, "sample unroll error");
	fail_unless(s.data[202] == s.data[201], "sample adjust error");
	fail_unless(s.data[203] == s.data[0], "sample adjust error");

}
END_TEST

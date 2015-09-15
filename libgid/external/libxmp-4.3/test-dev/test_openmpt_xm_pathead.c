#include "test.h"

/*
 Header size fields in XM files should be respected in all cases, even if
 they are too small or too big. In this case, the pattern header is not the
 usual 9 bytes, and the pattern header size indicates this. You have to skip
 as many bytes as this field says before starting to read the pattern.
*/

TEST(test_openmpt_xm_pathead)
{
	compare_mixer_data(
		"openmpt/xm/pathead.xm",
		"openmpt/xm/pathead.data");
}
END_TEST

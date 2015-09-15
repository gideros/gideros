#include "test.h"

/*
 This is the same as noteoff2.it, but with old effects enabled. In this
 case, the sample should never fade out.
*/

TEST(test_openmpt_it_noteoff3)
{
	compare_mixer_data(
		"openmpt/it/noteoff3.it",
		"openmpt/it/noteoff3.data");
}
END_TEST

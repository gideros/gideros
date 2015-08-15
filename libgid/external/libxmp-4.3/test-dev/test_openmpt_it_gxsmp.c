#include "test.h"

/*
 Another test case with an empty sample map slot which is simply ignored by
 Impulse Tracker.
*/

TEST(test_openmpt_it_gxsmp)
{
	compare_mixer_data_no_rv(
		"openmpt/it/gxsmp.it",
		"openmpt/it/gxsmp.data");
}
END_TEST

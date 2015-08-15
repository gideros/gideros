#include "test.h"

/*
 Going one step further by also changing the sample next to that portamento.
*/

TEST(test_openmpt_it_gxsmp2)
{
	compare_mixer_data_no_rv(
		"openmpt/it/gxsmp2.it",
		"openmpt/it/gxsmp2.data");
}
END_TEST

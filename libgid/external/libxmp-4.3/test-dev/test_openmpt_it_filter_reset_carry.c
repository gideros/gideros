#include "test.h"

/*
 I think this is also just an extension to the previous test case, to make
 sure that it does not break anything else.
*/

TEST(test_openmpt_it_filter_reset_carry)
{
	compare_mixer_data(
		"openmpt/it/filter-reset-carry.it",
		"openmpt/it/filter-reset-carry.data");
}
END_TEST

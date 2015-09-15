#include "test.h"

/*
 This test is just there to be sure that the filter-reset.it and
 filter-reset-carry.it test cases do not break NNA background channels.
*/

TEST(test_openmpt_it_filter_nna)
{
	compare_mixer_data(
		"openmpt/it/filter-nna.it",
		"openmpt/it/filter-nna.data");
}
END_TEST

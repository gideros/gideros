#include "test.h"

/*
 If resonant filters are rendered with integer arithmetic, they may produce
 scratching noises in some edge cases. You should not hear any scratches or
 other weird noises when playing this example.
*/

TEST(test_openmpt_it_extreme_filter_test_1)
{
	compare_mixer_data(
		"openmpt/it/extreme-filter-test-1.it",
		"openmpt/it/extreme-filter-test-1.data");
}
END_TEST

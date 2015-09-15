#include "test.h"

/*
 A small test case that demonstrates that full cutoff should not enable the
 filter if no resonance is applied. Resonance is only ever applied if the
 cutoff is not full or the resonance is not zero.
*/

TEST(test_openmpt_it_filter_7f)
{
	compare_mixer_data(
		"openmpt/it/filter-7F.it",
		"openmpt/it/filter-7F.data");
}
END_TEST

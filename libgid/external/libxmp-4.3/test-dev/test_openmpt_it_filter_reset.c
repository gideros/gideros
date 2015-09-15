#include "test.h"

/*
 As mentioned already, filtering is only ever done in IT if either cutoff is
 not full or if resonance is set. When a Z7F command is found next to a note
 and no portamento is applied, it disables the filter, however in other cases
 this should not happen.
*/

TEST(test_openmpt_it_filter_reset)
{
	compare_mixer_data(
		"openmpt/it/filter-reset.it",
		"openmpt/it/filter-reset.data");
}
END_TEST

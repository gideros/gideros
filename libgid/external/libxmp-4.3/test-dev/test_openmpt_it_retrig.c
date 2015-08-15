#include "test.h"

/*
 A combination of the retrigger effect and instrument envelopes. Not sure
 how this works.
*/

TEST(test_openmpt_it_retrig)
{
	compare_mixer_data(
		"openmpt/it/retrig.it",
		"openmpt/it/retrig.data");
}
END_TEST

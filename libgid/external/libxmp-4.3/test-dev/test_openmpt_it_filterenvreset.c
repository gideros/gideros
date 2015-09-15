#include "test.h"

/*
 A cutoff value of 0 should not be reset to full cutoff when triggering a
 note just because the filter envelope is enabled. This bug is probably very
 specific to OpenMPT, because it gets rid of some unneccessary code.
*/

TEST(test_openmpt_it_filterenvreset)
{
	compare_mixer_data(
		"openmpt/it/FilterEnvReset.it",
		"openmpt/it/FilterEnvReset.data");
}
END_TEST

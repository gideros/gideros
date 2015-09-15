#include "test.h"

/*
 Impulse Tracker resets envelopes under some more or less weird conditions.
*/

TEST(test_openmpt_it_envreset)
{
	compare_mixer_data(
		"openmpt/it/EnvReset.it",
		"openmpt/it/EnvReset.data");
}
END_TEST

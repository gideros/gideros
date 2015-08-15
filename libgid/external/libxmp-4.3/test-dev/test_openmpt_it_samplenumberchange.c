#include "test.h"

/*
 Contrary to InstrumentNumberChange.it, even ModPlug Tracker 1.16 passes it.
*/

TEST(test_openmpt_it_samplenumberchange)
{
	compare_mixer_data(
		"openmpt/it/SampleNumberChange.it",
		"openmpt/it/SampleNumberChange.data");
}
END_TEST

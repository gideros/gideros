#include "test.h"

/*
 While Impulse Tracker cuts playing samples if it encounters an invalid
 sample number in sample mode, the same does not happen if we are in
 instrument mode.
*/

TEST(test_openmpt_it_instrumentnumberchange)
{
	compare_mixer_data(
		"openmpt/it/InstrumentNumberChange.it",
		"openmpt/it/InstrumentNumberChange.data");
}
END_TEST

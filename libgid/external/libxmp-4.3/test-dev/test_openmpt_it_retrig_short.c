#include "test.h"

/*
 Impulse Tracker does not retrigger notes that are shorter than the duration
 of a tick. One might argue that this is a bug in Impulse Tracker, but OpenMPT
 emulates it anyway.
*/

TEST(test_openmpt_it_retrig_short)
{
	compare_mixer_data(
		"openmpt/it/retrig-short.it",
		"openmpt/it/retrig-short.data");
}
END_TEST

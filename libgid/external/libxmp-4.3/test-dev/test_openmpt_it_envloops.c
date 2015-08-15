#include "test.h"

/*
 In this test, all possible combinations of the envelope sustain point and
 envelope loops are tested, and you can see their behaviour on note-off. If
 the sustain point is at the loop end and the sustain loop has been released,
 don't loop anymore. Probably the most important thing for this test is that
 in Impulse Tracker (and Fasttracker 2), envelope position is incremented
 before the point is evaluated, not afterwards, so when no ticks have been
 processed yet, the envelope position should be invalid.
*/

TEST(test_openmpt_it_envloops)
{
	compare_mixer_data(
		"openmpt/it/EnvLoops.it",
		"openmpt/it/EnvLoops.data");
}
END_TEST

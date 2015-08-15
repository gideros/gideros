#include "test.h"

/*
 The S77 / S79 / S7B commands pause the instrument envelopes, they do not
 turn them off. S78 / S7A / S7C should resume the envelope at exactly the
 position where it was paused. In this test, it is again very important that
 the envelope position is incremented before the point is evaluated, not
 afterwards (see EnvLoops.it).
*/

TEST(test_openmpt_it_s77)
{
	compare_mixer_data(
		"openmpt/it/s77.it",
		"openmpt/it/s77.data");
}
END_TEST

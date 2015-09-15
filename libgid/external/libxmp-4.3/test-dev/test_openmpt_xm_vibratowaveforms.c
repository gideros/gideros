#include "test.h"

/*
 Fasttracker 2 tries to be inconsistent where possible, so you have to
 duplicate a lot of code or add conditions to behave exactly like Fasttracker.
 Yeah! Okay, seriously: Generally the vibrato and tremolo tables are identical
 to those that ProTracker uses, but the vibrato’s “ramp down” table is upside
 down. You will have to negate its sign for it to work as intended.
*/

TEST(test_openmpt_xm_vibratowaveforms)
{
	compare_mixer_data(
		"openmpt/xm/VibratoWaveforms.xm",
		"openmpt/xm/VibratoWaveforms.data");
}
END_TEST

#include "test.h"

/*
 If there are multiple pattern delays (EEx), only the one on the rightmost
 channel is considered (even if the EEx parameter is 0). Even ModPlug Tracker
 1.16 passes this. The second pattern is not very important, it only tests
 the command X ModPlug Tracker extension, which adds fine pattern delays
 (like in the IT format) to XM files.
*/

TEST(test_openmpt_xm_patterndelays)
{
	compare_mixer_data(
		"openmpt/xm/PatternDelays.xm",
		"openmpt/xm/PatternDelays.data");
}
END_TEST

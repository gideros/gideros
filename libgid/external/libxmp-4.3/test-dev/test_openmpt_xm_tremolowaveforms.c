#include "test.h"

/*
 OpenMPT does not process the ramp down waveform like Fasttracker 2.
*/

TEST(test_openmpt_xm_tremolowaveforms)
{
	compare_mixer_data(
		"openmpt/xm/TremoloWaveforms.xm",
		"openmpt/xm/TremoloWaveforms.data");
}
END_TEST

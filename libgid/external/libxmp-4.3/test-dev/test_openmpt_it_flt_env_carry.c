#include "test.h"

/*
 Envelope carry on the filter envelope. I think this is just a general test
 on how envelope carry is applied. It is possible that Impulse Tracker’s MMX
 drivers will play this in a different way from the WAV writer.
*/

TEST(test_openmpt_it_flt_env_carry)
{
	compare_mixer_data(
		"openmpt/it/flt-env-carry.it",
		"openmpt/it/flt-env-carry.data");
}
END_TEST

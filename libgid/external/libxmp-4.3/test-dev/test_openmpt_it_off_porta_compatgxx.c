#include "test.h"

/*
 When "Compatible Gxx" is enabled, the key-off flag should also be removed
 when continuing a note using a portamento command (row 2, 4, 6). This test
 case was written to discover a code regression when fixing Off-Porta.it).
*/

TEST(test_openmpt_it_off_porta_compatgxx)
{
	compare_mixer_data(
		"openmpt/it/Off-Porta-CompatGxx.it",
		"openmpt/it/Off-Porta-CompatGxx.data");
}
END_TEST

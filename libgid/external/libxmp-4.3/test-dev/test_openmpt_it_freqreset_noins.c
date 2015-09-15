#include "test.h"

/*
 When using multisample instruments, even notes with no instrument number
 next to them can change the sample (based on the active instrument’s sample
 map). When switching between samples, you must not forget to update the C-5
 frequency of the playing sample as well.
*/

TEST(test_openmpt_it_freqreset_noins)
{
	compare_mixer_data(
		"openmpt/it/freqreset-noins.it",
		"openmpt/it/freqreset-noins.data");
}
END_TEST

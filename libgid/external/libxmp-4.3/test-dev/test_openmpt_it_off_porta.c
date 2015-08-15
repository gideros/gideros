#include "test.h"

/*
 When "Compatible Gxx" is disabled, the key-off flag should only be removed
 when triggering new notes, but not when continuing a note using a portamento
 command (row 2, 4). However, you should keep in mind that the portamento flag
 is still set even if there is an offset command next to the portamento
 command (row 4), which would normally nullify the portamento effect (see
 porta-offset.it).
*/

TEST(test_openmpt_it_off_porta)
{
	compare_mixer_data(
		"openmpt/it/Off-Porta.it",
		"openmpt/it/Off-Porta.data");
}
END_TEST

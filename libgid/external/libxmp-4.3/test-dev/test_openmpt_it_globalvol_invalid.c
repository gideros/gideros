#include "test.h"

/*
 Out-of-range global volume commands (V81...VFF) should not change the current
 global volume. This test module should remain completely silent.
*/

TEST(test_openmpt_it_globalvol_invalid)
{
	compare_mixer_data(
		"openmpt/it/globalvol-invalid.it",
		"openmpt/it/globalvol-invalid.data");
}
END_TEST

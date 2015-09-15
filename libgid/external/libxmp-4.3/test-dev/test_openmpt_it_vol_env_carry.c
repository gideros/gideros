#include "test.h"

/*
 Similar to flt-env-carry.it.
*/

TEST(test_openmpt_it_vol_env_carry)
{
	compare_mixer_data(
		"openmpt/it/vol-env-carry.it",
		"openmpt/it/vol-env-carry.data");
}
END_TEST

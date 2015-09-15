#include "test.h"

/*
 Instrument filter settings should not be applied if there is a portamento
 effect.
*/

TEST(test_openmpt_it_ins_flt_porta_reset)
{
	compare_mixer_data(
		"openmpt/it/ins-flt-porta-reset.it",
		"openmpt/it/ins-flt-porta-reset.data");
}
END_TEST

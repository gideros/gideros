#include "test.h"

/*
 And another similar test.
*/

TEST(test_openmpt_it_gxxtest)
{
	compare_mixer_data(
		"openmpt/it/gxxtest.it",
		"openmpt/it/gxxtest.data");
}
END_TEST

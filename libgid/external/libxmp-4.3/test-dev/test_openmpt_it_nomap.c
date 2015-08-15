#include "test.h"

/*
 Yet another sample map test case.
 (check NNA on invalid sample mapping)
*/

TEST(test_openmpt_it_nomap)
{
	compare_mixer_data("openmpt/it/NoMap.it", "openmpt/it/NoMap.data");
}
END_TEST

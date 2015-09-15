#include "test.h"

/*
 I think, Impulse Tracker treats instruments like an additional layer of
 abstraction and first replaces the note and instrument in the pattern by
 the sample and note assignments from the sample map table before further
 evaluating the pattern. That would explain why for example the empty sample
 map slots do nothing in this module.
*/

TEST(test_openmpt_it_emptyslot)
{
	compare_mixer_data(
		"openmpt/it/emptyslot.it",
		"openmpt/it/emptyslot.data");
}
END_TEST

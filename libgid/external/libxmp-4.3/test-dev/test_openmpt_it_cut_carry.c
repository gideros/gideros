#include "test.h"

/*
 Envelopes that have the carry flag set cannot be “picked up” / continued
 after the note has been cut.
*/

TEST(test_openmpt_it_cut_carry)
{
	compare_mixer_data(
		"openmpt/it/cut-carry.it",
		"openmpt/it/cut-carry.data");
}
END_TEST

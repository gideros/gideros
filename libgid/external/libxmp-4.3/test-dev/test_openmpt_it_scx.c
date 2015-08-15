#include "test.h"

/*
 The SCx command cuts notes just like a normal note cut (^^^), it does not
 simply mute them. However, there is a difference when placing a lone
 instrument number after a note that was cut with SCx and one cut with ^^^,
 as it can be seen in this test case.
*/

TEST(test_openmpt_it_scx)
{
	compare_mixer_data(
		"openmpt/it/scx.it",
		"openmpt/it/scx.data");
}
END_TEST

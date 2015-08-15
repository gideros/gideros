#include "test.h"

/*
 Key off at tick 0 (K00) is very dodgy command. If there is a note next to it,
 the note is ignored. If there is a volume column command or instrument next
 to it and the current instrument has no volume envelope, the note is faded
 out instead of being cut.
*/

TEST(test_openmpt_xm_key_off)
{
	compare_mixer_data("openmpt/xm/key_off.xm", "openmpt/xm/key_off.data");
}
END_TEST

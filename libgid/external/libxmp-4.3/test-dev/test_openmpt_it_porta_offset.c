#include "test.h"
#include "../src/virtual.h"

/*
 Unlike Fasttracker 2, Impulse Tracker ignores the portamento command if
 there is an portamento command next to an offset command. Even ModPlug
 Tracker 1.16 gets this test right.
*/

TEST(test_openmpt_it_porta_offset)
{
	compare_mixer_data(
		"openmpt/it/porta-offset.it",
		"openmpt/it/porta-offset.data");
}
END_TEST

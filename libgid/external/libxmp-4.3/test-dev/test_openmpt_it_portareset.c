#include "test.h"

/*
 Impulse Tracker completely resets the portamento target on every new
 non-portamento note, i.e. a follow-up Gxx effect should not slide back to
 the previously triggered note.
*/

TEST(test_openmpt_it_portareset)
{
	compare_mixer_data(
		"openmpt/it/PortaReset.it",
		"openmpt/it/PortaReset.data");
}
END_TEST

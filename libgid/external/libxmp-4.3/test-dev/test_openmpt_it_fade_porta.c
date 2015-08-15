#include "test.h"

/*
 After a note has been stopped in some way (for example through fade-out or
 note cut), tone portamento effects on the following note are ignored, i.e.
 there is no portamento from the stopped note to the new note.
*/

TEST(test_openmpt_it_fade_porta)
{
	compare_mixer_data(
		"openmpt/it/Fade-Porta.it",
		"openmpt/it/Fade-Porta.data");
}
END_TEST

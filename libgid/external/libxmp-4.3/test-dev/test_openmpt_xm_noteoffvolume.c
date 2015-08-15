#include "test.h"

/*
 If an instrument has no volume envelope, a note-off command should cut the
 sample completely - unless there is a volume command next it. This applies
 to both volume commands (volume and effect column).
*/

TEST(test_openmpt_xm_noteoffvolume)
{
	compare_mixer_data(
		"openmpt/xm/NoteOffVolume.xm",
		"openmpt/xm/NoteOffVolume.data");
}
END_TEST

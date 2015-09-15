#include "test.h"

/*
 I think one of the first things Fasttracker 2 does when parsing a pattern
 cell is calculating the “real” note (i.e. pattern note + sample transpose),
 and if this “real” note falls out of its note range, it is ignored completely
 (wiped from its internal channel memory). The instrument number next it,
 however, is not affected and remains in the memory.
*/

TEST(test_openmpt_xm_notelimit)
{
	compare_mixer_data(
		"openmpt/xm/NoteLimit.xm",
		"openmpt/xm/NoteLimit.data");
}
END_TEST

#include "test.h"

/*
 Rogue note delay test. It seems that internally, Fasttracker 2 always acts
 like the last played note is next to a note delay (EDx with x > 0) if there
 is no note. Doing exactly this is probably the easiest way to pass this test.
 This also explains Fasttracker 2’s behaviour if there is an instrument number
 next to such a rogue note delay, which is shown in this test. Both channels
 should play exactly the same combination of snare and bass sounds.
*/

TEST(test_openmpt_xm_delay2)
{
	compare_mixer_data("openmpt/xm/delay2.xm", "openmpt/xm/delay2.data");
}
END_TEST

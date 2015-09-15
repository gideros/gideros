#include "test.h"

/*
 Note Delays combined with anything else are a lot of fun! Note Off combined
 with a Note Delay will cause envelopes to retrigger! And that is actually
 all it does if there is an envelope. No fade out, no nothing.
*/

/* Claudio's note -- I didn't implement the envelope retrigger thing, but
 * the test works nontheless. Maybe I'm doing something wrong?
 */

TEST(test_openmpt_xm_offdelay)
{
	compare_mixer_data(
		"openmpt/xm/OffDelay.xm",
		"openmpt/xm/OffDelay.data");
}
END_TEST

#include "test.h"

/*
 The interpretation of this scenario highly differs between various replayers.
 If the sample number next to a portamento effect differs from the previous
 number, the new sample's default volume should be applied and

 o the old sample should be played until reaching its end or loop end
   (ProTracker 1/2). If the sample loops, the new sample should be activated
   after the loop ended.

 o the new sample should be applied (ProTracker 3, various other players)

 OpenMPT implements the second option, which is also how it works in e.g. XM
 and S3M files.
*/

TEST(test_openmpt_mod_portasmpchange)
{
	compare_mixer_data(
		"openmpt/mod/PortaSmpChange.mod",
		"openmpt/mod/PortaSmpChange.data");
}
END_TEST

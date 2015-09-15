#include "test.h"

/*
 Just when you thought that you have got it right, you step into the next
 pitfall. Here you can see that the rogue note delay behaviour is only found
 with EDx effects with x > 0. An ED0 effect should just be ignored and not
 retrigger any notes. 
*/

TEST(test_openmpt_xm_delay3)
{
	compare_mixer_data("openmpt/xm/delay3.xm", "openmpt/xm/delay3.data");
}
END_TEST

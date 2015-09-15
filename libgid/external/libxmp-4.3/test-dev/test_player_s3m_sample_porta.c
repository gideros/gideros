#include "test.h"

/*
*/

TEST(test_player_s3m_sample_porta)
{
	compare_mixer_data(
		"data/s3m_sample_porta.s3m",
		"data/s3m_sample_porta.data");
}
END_TEST

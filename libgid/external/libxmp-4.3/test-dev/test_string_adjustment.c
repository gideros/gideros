#include "test.h"

TEST(test_string_adjustment)
{
	char string[80] = { 'h', 'e', 'l', 'l', 'o', 1, 2, 30, 31, 127,
		(char)128, 'w', 'o', 'r', 'l', 'd', ' ', ' ', ' ', 0 };

	adjust_string(string);
	fail_unless(strcmp(string, "hello      world") == 0, "adjustment error");
}
END_TEST

/*
 * Win32 debug message helper by Mirko Buffoni
 */

#if defined WIN32 && defined _DEBUG

#include "stdio2.h"
#include <stdarg.h>
void _D(const char *format, ...)
{
	va_list argptr;

	/* do the output */
	va_start(argptr, format);
	vprintf(format, argptr);
	printf("\n");
	va_end(argptr);
}

#endif

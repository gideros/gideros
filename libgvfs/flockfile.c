/*	$OpenBSD: flockfile.c,v 1.7 2004/09/28 18:12:44 otto Exp $	*/

#ifndef WINSTORE
#include <sys/time.h>
#endif

#include <stdio.h>
#include "local.h"

/*
 * Subroutine versions of the macros in <stdio.h>
 * Note that these are all no-ops because libc does not do threads.
 * Strong implementation of file locking in libc_r/uthread/uthread_file.c
 */

void
flockfile(FILE * fp)
{
}


int
ftrylockfile(FILE *fp)
{

	return 0;
}

void
funlockfile(FILE * fp)
{
}

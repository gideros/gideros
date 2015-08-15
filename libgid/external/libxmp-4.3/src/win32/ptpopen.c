/*
 * pt_popen/pt_pclose functions
 * Written somewhere in the 90s by Kurt Keller
 * Comments translated by Steve Donovan
 * Modified for use in xmp by Mirko Buffoni and Claudio Matsuoka
 */

/*
 * This piece of code is in the public domain. I do not claim any rights
 * on it. Do whatever you want to do with it and I hope it will be still
 * useful. -- Kurt Keller, Aug 2013
 */

#if !defined(HAVE_POPEN) && defined(WIN32)

#include "ptpopen.h"

/*
> Hello,
>      I am currently porting a UNIX program to WINDOWS.
> Most difficulty time I have is to find the popen()-like function under
> WINDOWS. Any help and hints would be greatly appreciated.
>
> Thanks in advance
> Tianlin Wang

This is what I use instead of popen(): (Sorry for the comments in german ;-))
It is not an **EXACT** replacement for popen() but it is OK for me.

Kurt.

---------------------------------------------------
Tel.:   (49)7150/393394    Parity Software GmbH
Fax.:   (49)7150/393351    Stuttgarter Strasse 42/3
E-Mail: kk@parity-soft.de  D-71701 Schwieberdingen
Web:    www.parity-soft.de
---------------------------------------------------
*/

/*----------------------------------------------------------------------------
  Globals for the Routines pt_popen() / pt_pclose()
----------------------------------------------------------------------------*/
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <errno.h>


static HANDLE my_pipein[2], my_pipeout[2], my_pipeerr[2];
static char my_popenmode = ' ';

static int my_pipe(HANDLE *readwrite)
{
  SECURITY_ATTRIBUTES sa;

  sa.nLength = sizeof(sa);          /* Length in bytes */
  sa.bInheritHandle = 1;            /* the child must inherit these handles */
  sa.lpSecurityDescriptor = NULL;

  if (! CreatePipe (&readwrite[0],&readwrite[1],&sa,1 << 13))
  {
    errno = EMFILE;
    return -1;
  }

  return 0;
}


/*----------------------------------------------------------------------------
  Replacement for 'popen()' under WIN32.
  NOTE: if cmd contains '2>&1', we connect the standard error file handle
    to the standard output file handle.
----------------------------------------------------------------------------*/
FILE * pt_popen(const char *cmd, const char *mode)
{
  FILE *fptr = (FILE *)0;
  PROCESS_INFORMATION piProcInfo;
  STARTUPINFO siStartInfo;
  int success, umlenkung;

  my_pipein[0]   = INVALID_HANDLE_VALUE;
  my_pipein[1]   = INVALID_HANDLE_VALUE;
  my_pipeout[0]  = INVALID_HANDLE_VALUE;
  my_pipeout[1]  = INVALID_HANDLE_VALUE;
  my_pipeerr[0]  = INVALID_HANDLE_VALUE;
  my_pipeerr[1]  = INVALID_HANDLE_VALUE;

  if (!mode || !*mode)
    goto finito;

  my_popenmode = *mode;
  if (my_popenmode != 'r' && my_popenmode != 'w')
    goto finito;

  /*
   * Shall we redirect stderr to stdout ? */
  umlenkung = strstr("2>&1",(char *)cmd) != 0;

  /*
   * Create the Pipes... */
  if (my_pipe(my_pipein)  == -1 ||
      my_pipe(my_pipeout) == -1)
    goto finito;
  if (!umlenkung && my_pipe(my_pipeerr) == -1)
    goto finito;

  /*
   * Now create the child process */
  ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
  siStartInfo.cb           = sizeof(STARTUPINFO);
  siStartInfo.hStdInput    = my_pipein[0];
  siStartInfo.hStdOutput   = my_pipeout[1];
  if (umlenkung)
    siStartInfo.hStdError  = my_pipeout[1];
  else
    siStartInfo.hStdError  = my_pipeerr[1];
  siStartInfo.dwFlags    = STARTF_USESTDHANDLES;

  success = CreateProcess(NULL,
     (LPTSTR)cmd,       // command line
     NULL,              // process security attributes
     NULL,              // primary thread security attributes
     TRUE,              // handles are inherited
     DETACHED_PROCESS,  // creation flags: without window (?)
     NULL,              // use parent's environment
     NULL,              // use parent's current directory
     &siStartInfo,      // STARTUPINFO pointer
     &piProcInfo);      // receives PROCESS_INFORMATION

  if (!success)
    goto finito;

  /*
   * These handles listen to the child process */
  CloseHandle(my_pipein[0]);  my_pipein[0]  = INVALID_HANDLE_VALUE;
  CloseHandle(my_pipeout[1]); my_pipeout[1] = INVALID_HANDLE_VALUE;
  CloseHandle(my_pipeerr[1]); my_pipeerr[1] = INVALID_HANDLE_VALUE;

  if (my_popenmode == 'r')
    fptr = _fdopen(_open_osfhandle((long)my_pipeout[0],_O_BINARY),"r");
  else
    fptr = _fdopen(_open_osfhandle((long)my_pipein[1],_O_BINARY),"w");

finito:
  if (!fptr)
  {
    if (my_pipein[0]  != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipein[0]);
    if (my_pipein[1]  != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipein[1]);
    if (my_pipeout[0] != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipeout[0]);
    if (my_pipeout[1] != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipeout[1]);
    if (my_pipeerr[0] != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipeerr[0]);
    if (my_pipeerr[1] != INVALID_HANDLE_VALUE)
      CloseHandle(my_pipeerr[1]);
  }
  return fptr;
}

/*----------------------------------------------------------------------------
  Replacement for 'pclose()' under WIN32
----------------------------------------------------------------------------*/
int pt_pclose(FILE *fle)
{
  if (fle)
  {
    (void)fclose(fle);

    CloseHandle(my_pipeerr[0]);
    if (my_popenmode == 'r')
      CloseHandle(my_pipein[1]);
    else
     CloseHandle(my_pipeout[0]);
    return 0;
  }
  return -1;
}

#endif /* !HAVE_POPEN && WIN32 */

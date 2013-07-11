/*
 * pthread_once.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2005 Pthreads-win32 contributors
 * 
 *      Contact Email: rpj@callisto.canberra.edu.au
 * 
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "pthread.h"
#include "implement.h"


static void PTW32_CDECL
ptw32_once_init_routine_cleanup(void * arg)
{
  pthread_once_t * once_control = (pthread_once_t *) arg;

  /*
   * Continue to direct new threads into the wait path until the waiter that we
   * release can reset state to INIT.
   */
  (void) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->state, (LONG)PTW32_ONCE_CANCELLED);

  if (InterlockedExchangeAdd((LPLONG)&once_control->semaphore, 0L)) /* MBR fence */
    {
      ReleaseSemaphore(once_control->semaphore, 1, NULL);
    }
}

int
pthread_once (pthread_once_t * once_control, void (*init_routine) (void))
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      If any thread in a process  with  a  once_control  parameter
      *      makes  a  call to pthread_once(), the first call will summon
      *      the init_routine(), but  subsequent  calls  will  not. The
      *      once_control  parameter  determines  whether  the associated
      *      initialization routine has been called.  The  init_routine()
      *      is complete upon return of pthread_once().
      *      This function guarantees that one and only one thread
      *      executes the initialization routine, init_routine when
      *      access is controlled by the pthread_once_t control
      *      key.
      *
      *      pthread_once() is not a cancelation point, but the init_routine
      *      can be. If it's cancelled then the effect on the once_control is
      *      as if pthread_once had never been entered.
      *
      *
      * PARAMETERS
      *      once_control
      *              pointer to an instance of pthread_once_t
      *
      *      init_routine
      *              pointer to an initialization routine
      *
      *
      * DESCRIPTION
      *      See above.
      *
      * RESULTS
      *              0               success,
      *              EINVAL          once_control or init_routine is NULL
      *
      * ------------------------------------------------------
      */
{
  int result;
  int state;
  HANDLE sema;

  if (once_control == NULL || init_routine == NULL)
    {
      result = EINVAL;
      goto FAIL0;
    }
  else
    {
      result = 0;
    }

  while ((state = (int)
	  PTW32_INTERLOCKED_COMPARE_EXCHANGE((PTW32_INTERLOCKED_LPLONG)&once_control->state,
					     (PTW32_INTERLOCKED_LONG)PTW32_ONCE_STARTED,
					     (PTW32_INTERLOCKED_LONG)PTW32_ONCE_INIT))
	 != PTW32_ONCE_DONE)
    {
      if (PTW32_ONCE_INIT == state)
        {

#ifdef _MSC_VER
#pragma inline_depth(0)
#endif

          pthread_cleanup_push(ptw32_once_init_routine_cleanup, (void *) once_control);
          (*init_routine)();
          pthread_cleanup_pop(0);

#ifdef _MSC_VER
#pragma inline_depth()
#endif

          (void) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->state, 
                                            (LONG)PTW32_ONCE_DONE);

          /*
           * we didn't create the semaphore.
           * it is only there if there is someone waiting.
           */
          if (InterlockedExchangeAdd((LPLONG)&once_control->semaphore, 0L)) /* MBR fence */
            {
              ReleaseSemaphore(once_control->semaphore, 
                               once_control->numSemaphoreUsers, NULL);
            }
        }
      else
        {
          if (1 == InterlockedIncrement((LPLONG)&once_control->numSemaphoreUsers))

//          if (!InterlockedExchangeAdd((LPLONG)&once_control->semaphore, 0L)) /* MBR fence */
            {
              sema = CreateSemaphore(NULL, 0, INT_MAX, NULL);

              if (PTW32_INTERLOCKED_COMPARE_EXCHANGE((PTW32_INTERLOCKED_LPLONG)&once_control->semaphore,
						     (PTW32_INTERLOCKED_LONG)sema,
						     (PTW32_INTERLOCKED_LONG)0))
                {
                  CloseHandle(sema);
                }
            }

	  /*
	   * If initter was cancelled then state is CANCELLED.
	   * Until state is reset to INIT, all new threads will enter the wait path.
	   * The woken waiter, if it exists, will also re-enter the wait path, but
	   * either it or a new thread will reset state = INIT here, continue around the Wait,
	   * and become the new initter. Any thread that is suspended in the wait path before
	   * this point will hit this check. Any thread suspended between this check and
	   * the Wait will wait on a valid semaphore, and possibly continue through it
	   * if the cancellation handler has incremented (released) it and there were
	   * no waiters.
	   */
	  (void) PTW32_INTERLOCKED_COMPARE_EXCHANGE((PTW32_INTERLOCKED_LPLONG)&once_control->state,
						    (PTW32_INTERLOCKED_LONG)PTW32_ONCE_INIT,
						    (PTW32_INTERLOCKED_LONG)PTW32_ONCE_CANCELLED);

          /*
           * Check 'state' again in case the initting thread has finished
	   * and left before seeing that there was a semaphore.
	   */
          if (InterlockedExchangeAdd((LPLONG)&once_control->state, 0L) >= PTW32_ONCE_STARTED)
            {
              WaitForSingleObject(once_control->semaphore, INFINITE);
            }

          if (0 == InterlockedDecrement((LPLONG)&once_control->numSemaphoreUsers))
            {
              /* we were last */
              if ((sema =
		   (HANDLE) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->semaphore, (LONG)0)))
                {
                  CloseHandle(sema);
                }
            }
        }
    }

  /*
   * ------------
   * Failure Code
   * ------------
   */
FAIL0:
  return (result);
}                               /* pthread_once */

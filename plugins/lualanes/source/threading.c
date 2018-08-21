/*
 * THREADING.C                        Copyright (c) 2007-08, Asko Kauppi
 *                                    Copyright (C) 2009-14, Benoit Germain
 *
 * Lua Lanes OS threading specific code.
 *
 * References:
 *      <http://www.cse.wustl.edu/~schmidt/win32-cv-1.html>
*/

/*
===============================================================================

Copyright (C) 2007-10 Asko Kauppi <akauppi@gmail.com>
Copyright (C) 2009-14, Benoit Germain <bnt.germain@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

===============================================================================
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <math.h>

#include "threading.h"

#if !defined( PLATFORM_XBOX) && !defined( PLATFORM_WIN32) && !defined( PLATFORM_POCKETPC)
# include <sys/time.h>
#endif // non-WIN32 timing


#if defined(PLATFORM_LINUX) || defined(PLATFORM_CYGWIN)
# include <sys/types.h>
# include <unistd.h>
#endif

/* Linux needs to check, whether it's been run as root
*/
#ifdef PLATFORM_LINUX
  volatile bool_t sudo;
#endif

/* Linux with older glibc (such as Debian) don't have pthread_setname_np, but have prctl
*/
#if defined PLATFORM_LINUX
#if defined __GNU_LIBRARY__ && __GLIBC__ >= 2 && __GLIBC_MINOR__ >= 12
#define LINUX_USE_PTHREAD_SETNAME_NP 1
#else // glibc without pthread_setname_np
#include <sys/prctl.h>
#define LINUX_USE_PTHREAD_SETNAME_NP 0
#endif // glibc without pthread_setname_np
#endif // PLATFORM_LINUX

#ifdef _MSC_VER
// ".. selected for automatic inline expansion" (/O2 option)
# pragma warning( disable : 4711 )
// ".. type cast from function pointer ... to data pointer"
# pragma warning( disable : 4054 )
#endif

//#define THREAD_CREATE_RETRIES_MAX 20
    // loops (maybe retry forever?)

/* 
* FAIL is for unexpected API return values - essentially programming 
* error in _this_ code. 
*/
#if defined( PLATFORM_XBOX) || defined( PLATFORM_WIN32) || defined( PLATFORM_POCKETPC)
static void FAIL( char const* funcname, int rc)
{
#if defined( PLATFORM_XBOX)
	fprintf( stderr, "%s() failed! (%d)\n", funcname, rc );
#else // PLATFORM_XBOX
	char buf[256];
	FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM, NULL, rc, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
	fprintf( stderr, "%s() failed! [GetLastError() -> %d] '%s'", funcname, rc, buf);
#endif // PLATFORM_XBOX
#ifdef _MSC_VER
	__debugbreak(); // give a chance to the debugger!
#endif // _MSC_VER
	abort();
}
#endif // win32 build


/*
* Returns millisecond timing (in seconds) for the current time.
*
* Note: This function should be called once in single-threaded mode in Win32,
*       to get it initialized.
*/
time_d now_secs(void) {

#if defined( PLATFORM_XBOX) || defined( PLATFORM_WIN32) || defined( PLATFORM_POCKETPC)
    /*
    * Windows FILETIME values are "100-nanosecond intervals since 
    * January 1, 1601 (UTC)" (MSDN). Well, we'd want Unix Epoch as
    * the offset and it seems, so would they:
    *
    * <http://msdn.microsoft.com/en-us/library/ms724928(VS.85).aspx>
    */
    SYSTEMTIME st;
    FILETIME ft;
    ULARGE_INTEGER uli;
    static ULARGE_INTEGER uli_epoch;   // Jan 1st 1970 0:0:0

    if (uli_epoch.HighPart==0) {
        st.wYear= 1970;
        st.wMonth= 1;   // Jan
        st.wDay= 1;
        st.wHour= st.wMinute= st.wSecond= st.wMilliseconds= 0;

        if (!SystemTimeToFileTime( &st, &ft ))
            FAIL( "SystemTimeToFileTime", GetLastError() );

        uli_epoch.LowPart= ft.dwLowDateTime;
        uli_epoch.HighPart= ft.dwHighDateTime;
    }

    GetSystemTime( &st );	// current system date/time in UTC
    if (!SystemTimeToFileTime( &st, &ft ))
        FAIL( "SystemTimeToFileTime", GetLastError() );

    uli.LowPart= ft.dwLowDateTime;
    uli.HighPart= ft.dwHighDateTime;

    /* 'double' has less accuracy than 64-bit int, but if it were to degrade,
     * it would do so gracefully. In practice, the integer accuracy is not
     * of the 100ns class but just 1ms (Windows XP).
     */
# if 1
    // >= 2.0.3 code
    return (double) ((uli.QuadPart - uli_epoch.QuadPart)/10000) / 1000.0;
# elif 0
    // fix from Kriss Daniels, see: 
    // <http://luaforge.net/forum/forum.php?thread_id=22704&forum_id=1781>
    //
    // "seem to be getting negative numbers from the old version, probably number
    // conversion clipping, this fixes it and maintains ms resolution"
    //
    // This was a bad fix, and caused timer test 5 sec timers to disappear.
    // --AKa 25-Jan-2009
    //
    return ((double)((signed)((uli.QuadPart/10000) - (uli_epoch.QuadPart/10000)))) / 1000.0;
# else
    // <= 2.0.2 code
    return (double)(uli.QuadPart - uli_epoch.QuadPart) / 10000000.0;
# endif
#else // !(defined( PLATFORM_WIN32) || defined( PLATFORM_POCKETPC))
    struct timeval tv;
        // {
        //   time_t       tv_sec;   /* seconds since Jan. 1, 1970 */
        //   suseconds_t  tv_usec;  /* and microseconds */
        // };

    int rc= gettimeofday( &tv, NULL /*time zone not used any more (in Linux)*/ );
    assert( rc==0 );

    return ((double)tv.tv_sec) + ((tv.tv_usec)/1000) / 1000.0;
#endif // !(defined( PLATFORM_WIN32) || defined( PLATFORM_POCKETPC))
}


/*
*/
time_d SIGNAL_TIMEOUT_PREPARE( double secs ) {
    if (secs<=0.0) return secs;
    else return now_secs() + secs;
}


#if THREADAPI == THREADAPI_PTHREAD
/*
* Prepare 'abs_secs' kind of timeout to 'timespec' format
*/
static void prepare_timeout( struct timespec *ts, time_d abs_secs ) {
    assert(ts);
    assert( abs_secs >= 0.0 );

    if (abs_secs==0.0)
        abs_secs= now_secs();

    ts->tv_sec= (time_t) floor( abs_secs );
    ts->tv_nsec= ((long)((abs_secs - ts->tv_sec) * 1000.0 +0.5)) * 1000000UL;   // 1ms = 1000000ns
    if (ts->tv_nsec == 1000000000UL)
    {
        ts->tv_nsec = 0;
        ts->tv_sec = ts->tv_sec + 1;
    }
}
#endif // THREADAPI == THREADAPI_PTHREAD


/*---=== Threading ===---*/

//---
// It may be meaningful to explicitly limit the new threads' C stack size.
// We should know how much Lua needs in the C stack, all Lua side allocations
// are done in heap so they don't count.
//
// Consequence of _not_ limiting the stack is running out of virtual memory
// with 1000-5000 threads on 32-bit systems.
//
// Note: using external C modules may be affected by the stack size check.
//       if having problems, set back to '0' (default stack size of the system).
// 
// Win32:       64K (?)
// Win64:       xxx
//
// Linux x86:   2MB     Ubuntu 7.04 via 'pthread_getstacksize()'
// Linux x64:   xxx
// Linux ARM:   xxx
//
// OS X 10.4.9: 512K    <http://developer.apple.com/qa/qa2005/qa1419.html>
//                      valid values N * 4KB
//
#ifndef _THREAD_STACK_SIZE
# if defined( PLATFORM_XBOX) || defined( PLATFORM_WIN32) || defined( PLATFORM_POCKETPC) || defined( PLATFORM_CYGWIN)
#  define _THREAD_STACK_SIZE 0
      // Win32: does it work with less?
# elif (defined PLATFORM_OSX)
#  define _THREAD_STACK_SIZE (524288/2)   // 262144
      // OS X: "make test" works on 65536 and even below
      //       "make perftest" works on >= 4*65536 == 262144 (not 3*65536)
# elif (defined PLATFORM_LINUX) && (defined __i386)
#  define _THREAD_STACK_SIZE (2097152/16)  // 131072
      // Linux x86 (Ubuntu 7.04): "make perftest" works on /16 (not on /32)
# elif (defined PLATFORM_BSD) && (defined __i386)
#  define _THREAD_STACK_SIZE (1048576/8)  // 131072
      // FreeBSD 6.2 SMP i386: ("gmake perftest" works on /8 (not on /16)
# endif
#endif

#if THREADAPI == THREADAPI_WINDOWS

#if _WIN32_WINNT < 0x0600 // CONDITION_VARIABLE aren't available
  //
  void MUTEX_INIT( MUTEX_T *ref ) {
     *ref= CreateMutex( NULL /*security attr*/, FALSE /*not locked*/, NULL );
     if (!ref) FAIL( "CreateMutex", GetLastError() );
  }
  void MUTEX_FREE( MUTEX_T *ref ) {
     if (!CloseHandle(*ref)) FAIL( "CloseHandle (mutex)", GetLastError() );
     *ref= NULL;
  }
	void MUTEX_LOCK( MUTEX_T *ref )
	{
		DWORD rc = WaitForSingleObject( *ref, INFINITE);
		// ERROR_WAIT_NO_CHILDREN means a thread was killed (lane terminated because of error raised during a linda transfer for example) while having grabbed this mutex
		// this is not a big problem as we will grab it just the same, so ignore this particular error
		if( rc != 0 && rc != ERROR_WAIT_NO_CHILDREN)
			FAIL( "WaitForSingleObject", (rc == WAIT_FAILED) ? GetLastError() : rc);
	}
  void MUTEX_UNLOCK( MUTEX_T *ref ) {
    if (!ReleaseMutex(*ref))
        FAIL( "ReleaseMutex", GetLastError() );
  }
#endif // CONDITION_VARIABLE aren't available

static int const gs_prio_remap[] =
{
	THREAD_PRIORITY_IDLE,
	THREAD_PRIORITY_LOWEST,
	THREAD_PRIORITY_BELOW_NORMAL,
	THREAD_PRIORITY_NORMAL,
	THREAD_PRIORITY_ABOVE_NORMAL,
	THREAD_PRIORITY_HIGHEST,
	THREAD_PRIORITY_TIME_CRITICAL
};

/* MSDN: "If you would like to use the CRT in ThreadProc, use the
_beginthreadex function instead (of CreateThread)."
MSDN: "you can create at most 2028 threads"
*/
// Note: Visual C++ requires '__stdcall' where it is
void THREAD_CREATE( THREAD_T* ref, THREAD_RETURN_T (__stdcall *func)( void*), void* data, int prio /* -3..+3 */)
{
	HANDLE h = (HANDLE) _beginthreadex( NULL, // security
		_THREAD_STACK_SIZE,
		func,
		data,
		0,    // flags (0/CREATE_SUSPENDED)
		NULL  // thread id (not used)
	);

	if( h == NULL) // _beginthreadex returns 0L on failure instead of -1L (like _beginthread)
	{
		FAIL( "CreateThread", GetLastError());
	}

	if (!SetThreadPriority( h, gs_prio_remap[prio + 3]))
	{
		FAIL( "SetThreadPriority", GetLastError());
	}

	*ref = h;
}


void THREAD_SET_PRIORITY( int prio)
{
	// prio range [-3,+3] was checked by the caller
	if (!SetThreadPriority( GetCurrentThread(), gs_prio_remap[prio + 3]))
	{
		FAIL( "THREAD_SET_PRIORITY", GetLastError());
	}
}


bool_t THREAD_WAIT_IMPL( THREAD_T *ref, double secs)
{
    DWORD ms = (secs<0.0) ? INFINITE : (DWORD)((secs*1000.0)+0.5);

    DWORD rc= WaitForSingleObject( *ref, ms /*timeout*/ );
        //
        // (WAIT_ABANDONED)
        // WAIT_OBJECT_0    success (0)
        // WAIT_TIMEOUT
        // WAIT_FAILED      more info via GetLastError()

    if (rc == WAIT_TIMEOUT) return FALSE;
    if( rc !=0) FAIL( "WaitForSingleObject", rc==WAIT_FAILED ? GetLastError() : rc);
    *ref= NULL;     // thread no longer usable
    return TRUE;
  }
  //
	void THREAD_KILL( THREAD_T *ref )
	{
		// nonexistent on Xbox360, simply disable until a better solution is found
		#if !defined( PLATFORM_XBOX)
		// in theory no-one should call this as it is very dangerous (memory and mutex leaks, no notification of DLLs, etc.)
		if (!TerminateThread( *ref, 0 )) FAIL("TerminateThread", GetLastError());
		#endif // PLATFORM_XBOX
		*ref= NULL;
	}

	void THREAD_MAKE_ASYNCH_CANCELLABLE() {} // nothing to do for windows threads, we can cancel them anytime we want

#if !defined __GNUC__
	//see http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
	#define MS_VC_EXCEPTION 0x406D1388
	#pragma pack(push,8)
		typedef struct tagTHREADNAME_INFO
		{
			DWORD dwType; // Must be 0x1000.
			LPCSTR szName; // Pointer to name (in user addr space).
			DWORD dwThreadID; // Thread ID (-1=caller thread).
			DWORD dwFlags; // Reserved for future use, must be zero.
		} THREADNAME_INFO;
	#pragma pack(pop)
#endif // !__GNUC__

	void THREAD_SETNAME( char const* _name)
	{
#if !defined __GNUC__
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = _name;
		info.dwThreadID = GetCurrentThreadId();
		info.dwFlags = 0;

		__try
		{
			RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
#endif // !__GNUC__
	}

#if _WIN32_WINNT < 0x0600 // CONDITION_VARIABLE aren't available

	void SIGNAL_INIT( SIGNAL_T* ref)
	{
		InitializeCriticalSection( &ref->signalCS);
		InitializeCriticalSection( &ref->countCS);
		if( 0 == (ref->waitEvent = CreateEvent( 0, TRUE, FALSE, 0)))     // manual-reset
			FAIL( "CreateEvent", GetLastError());
		if( 0 == (ref->waitDoneEvent = CreateEvent( 0, FALSE, FALSE, 0)))    // auto-reset
			FAIL( "CreateEvent", GetLastError());
		ref->waitersCount = 0;
	}

	void SIGNAL_FREE( SIGNAL_T* ref)
	{
		CloseHandle( ref->waitDoneEvent);
		CloseHandle( ref->waitEvent);
		DeleteCriticalSection( &ref->countCS);
		DeleteCriticalSection( &ref->signalCS);
	}

	bool_t SIGNAL_WAIT( SIGNAL_T* ref, MUTEX_T* mu_ref, time_d abs_secs)
	{
		DWORD errc;
		DWORD ms;

		if( abs_secs < 0.0)
			ms = INFINITE;
		else if( abs_secs == 0.0)
			ms = 0;
		else 
		{
			time_d msd = (abs_secs - now_secs()) * 1000.0 + 0.5;
			// If the time already passed, still try once (ms==0). A short timeout
			// may have turned negative or 0 because of the two time samples done.
			ms = msd <= 0.0 ? 0 : (DWORD)msd;
		}

		EnterCriticalSection( &ref->signalCS);
		EnterCriticalSection( &ref->countCS);
		++ ref->waitersCount;
		LeaveCriticalSection( &ref->countCS);
		LeaveCriticalSection( &ref->signalCS);

		errc = SignalObjectAndWait( *mu_ref, ref->waitEvent, ms, FALSE);

		EnterCriticalSection( &ref->countCS);
		if( 0 == -- ref->waitersCount)
		{
			// we're the last one leaving...
			ResetEvent( ref->waitEvent);
			SetEvent( ref->waitDoneEvent);
		}
		LeaveCriticalSection( &ref->countCS);
		MUTEX_LOCK( mu_ref);

		switch( errc)
		{
			case WAIT_TIMEOUT:
			return FALSE;
			case WAIT_OBJECT_0:
			return TRUE;
		}

		FAIL( "SignalObjectAndWait", GetLastError());
		return FALSE;
	}

	void SIGNAL_ALL( SIGNAL_T* ref)
	{
		DWORD errc = WAIT_OBJECT_0;

		EnterCriticalSection( &ref->signalCS);
		EnterCriticalSection( &ref->countCS);

		if( ref->waitersCount > 0)
		{
			ResetEvent( ref->waitDoneEvent);
			SetEvent( ref->waitEvent);
			LeaveCriticalSection( &ref->countCS);
			errc = WaitForSingleObject( ref->waitDoneEvent, INFINITE);
		}
		else
		{
			LeaveCriticalSection( &ref->countCS);
		}

		LeaveCriticalSection( &ref->signalCS);

		if( WAIT_OBJECT_0 != errc)
			FAIL( "WaitForSingleObject", GetLastError());
	}

#else // CONDITION_VARIABLE are available, use them

	//
	void SIGNAL_INIT( SIGNAL_T *ref )
	{
		InitializeConditionVariable( ref);
	}

	void SIGNAL_FREE( SIGNAL_T *ref )
	{
		// nothing to do
		ref;
	}

	bool_t SIGNAL_WAIT( SIGNAL_T *ref, MUTEX_T *mu_ref, time_d abs_secs)
	{
		long ms;

		if( abs_secs < 0.0)
			ms = INFINITE;
		else if( abs_secs == 0.0)
			ms = 0;
		else
		{
			ms = (long) ((abs_secs - now_secs())*1000.0 + 0.5);

			// If the time already passed, still try once (ms==0). A short timeout
			// may have turned negative or 0 because of the two time samples done.
			//
			if( ms < 0)
				ms = 0;
		}

		if( !SleepConditionVariableCS( ref, mu_ref, ms))
		{
			if( GetLastError() == ERROR_TIMEOUT)
			{
				return FALSE;
			}
			else
			{
				FAIL( "SleepConditionVariableCS", GetLastError());
			}
		}
		return TRUE;
	}

	void SIGNAL_ONE( SIGNAL_T *ref )
	{
		WakeConditionVariable( ref);
	}

	void SIGNAL_ALL( SIGNAL_T *ref )
	{
		WakeAllConditionVariable( ref);
	}

#endif // CONDITION_VARIABLE are available

#else // THREADAPI == THREADAPI_PTHREAD
  // PThread (Linux, OS X, ...)
  //
  // On OS X, user processes seem to be able to change priorities.
  // On Linux, SCHED_RR and su privileges are required..  !-(
  //
  #include <errno.h>

#	if (defined(__MINGW32__) || defined(__MINGW64__)) && defined pthread_attr_setschedpolicy
#	if pthread_attr_setschedpolicy( A, S) == ENOTSUP
	// from the mingw-w64 team:
	// Well, we support pthread_setschedparam by which you can specify
	// threading-policy. Nevertheless, yes we lack this function. In
	// general its implementation is pretty much trivial, as on Win32 target
	// just SCHED_OTHER can be supported.
	#undef pthread_attr_setschedpolicy
	static int pthread_attr_setschedpolicy( pthread_attr_t* attr, int policy)
	{
		if( policy != SCHED_OTHER)
		{
			return ENOTSUP;
		}
		return 0;
	}
#	endif // pthread_attr_setschedpolicy()
#	endif // defined(__MINGW32__) || defined(__MINGW64__)

  static void _PT_FAIL( int rc, const char *name, const char *file, uint_t line ) {
    const char *why= (rc==EINVAL) ? "EINVAL" : 
                     (rc==EBUSY) ? "EBUSY" : 
                     (rc==EPERM) ? "EPERM" :
                     (rc==ENOMEM) ? "ENOMEM" :
                     (rc==ESRCH) ? "ESRCH" :
                     (rc==ENOTSUP) ? "ENOTSUP":
                     //...
                     "<UNKNOWN>";
    fprintf( stderr, "%s %d: %s failed, %d %s\n", file, line, name, rc, why );
    abort();
  }
  #define PT_CALL( call ) { int rc= call; if (rc!=0) _PT_FAIL( rc, #call, __FILE__, __LINE__ ); }
  //
  void SIGNAL_INIT( SIGNAL_T *ref ) {
    PT_CALL( pthread_cond_init(ref,NULL /*attr*/) );
    }
  void SIGNAL_FREE( SIGNAL_T *ref ) {
    PT_CALL( pthread_cond_destroy(ref) );
  }
  //
  /*
  * Timeout is given as absolute since we may have fake wakeups during
  * a timed out sleep. A Linda with some other key read, or just because
  * PThread cond vars can wake up unwantedly.
  */
  bool_t SIGNAL_WAIT( SIGNAL_T *ref, pthread_mutex_t *mu, time_d abs_secs ) {
    if (abs_secs<0.0) {
        PT_CALL( pthread_cond_wait( ref, mu ) );  // infinite
    } else {
        int rc;
        struct timespec ts;

        assert( abs_secs != 0.0 );
        prepare_timeout( &ts, abs_secs );

        rc= pthread_cond_timedwait( ref, mu, &ts );

        if (rc==ETIMEDOUT) return FALSE;
        if (rc) { _PT_FAIL( rc, "pthread_cond_timedwait()", __FILE__, __LINE__ ); }
    }
    return TRUE;
  }
  //
  void SIGNAL_ONE( SIGNAL_T *ref ) {
    PT_CALL( pthread_cond_signal(ref) );     // wake up ONE (or no) waiting thread
  }
  //
  void SIGNAL_ALL( SIGNAL_T *ref ) {
    PT_CALL( pthread_cond_broadcast(ref) );     // wake up ALL waiting threads
  }

// array of 7 thread priority values, hand-tuned by platform so that we offer a uniform [-3,+3] public priority range
static int const gs_prio_remap[] =
{
	// NB: PThreads priority handling is about as twisty as one can get it
	//     (and then some). DON*T TRUST ANYTHING YOU READ ON THE NET!!!

	//---
	// "Select the scheduling policy for the thread: one of SCHED_OTHER 
	// (regular, non-real-time scheduling), SCHED_RR (real-time, 
	// round-robin) or SCHED_FIFO (real-time, first-in first-out)."
	//
	// "Using the RR policy ensures that all threads having the same
	// priority level will be scheduled equally, regardless of their activity."
	//
	// "For SCHED_FIFO and SCHED_RR, the only required member of the
	// sched_param structure is the priority sched_priority. For SCHED_OTHER,
	// the affected scheduling parameters are implementation-defined."
	//
	// "The priority of a thread is specified as a delta which is added to 
	// the priority of the process."
	//
	// ".. priority is an integer value, in the range from 1 to 127. 
	//  1 is the least-favored priority, 127 is the most-favored."
	//
	// "Priority level 0 cannot be used: it is reserved for the system."
	//
	// "When you use specify a priority of -99 in a call to 
	// pthread_setschedparam(), the priority of the target thread is 
	// lowered to the lowest possible value."
	//
	// ...

	// ** CONCLUSION **
	//
	// PThread priorities are _hugely_ system specific, and we need at
	// least OS specific settings. Hopefully, Linuxes and OS X versions
	// are uniform enough, among each other...
	//
#	if defined PLATFORM_OSX
		// AK 10-Apr-07 (OS X PowerPC 10.4.9):
		//
		// With SCHED_RR, 26 seems to be the "normal" priority, where setting
		// it does not seem to affect the order of threads processed.
		//
		// With SCHED_OTHER, the range 25..32 is normal (maybe the same 26,
		// but the difference is not so clear with OTHER).
		//
		// 'sched_get_priority_min()' and '..max()' give 15, 47 as the 
		// priority limits. This could imply, user mode applications won't
		// be able to use values outside of that range.
		//
#			define _PRIO_MODE SCHED_OTHER

	// OS X 10.4.9 (PowerPC) gives ENOTSUP for process scope
			//#define _PRIO_SCOPE PTHREAD_SCOPE_PROCESS

#			define _PRIO_HI  32    // seems to work (_carefully_ picked!)
#			define _PRIO_0   26    // detected
#			define _PRIO_LO   1    // seems to work (tested)

#		elif defined PLATFORM_LINUX
			// (based on Ubuntu Linux 2.6.15 kernel)
			//
			// SCHED_OTHER is the default policy, but does not allow for priorities.
			// SCHED_RR allows priorities, all of which (1..99) are higher than
			// a thread with SCHED_OTHER policy.
			//
			// <http://kerneltrap.org/node/6080>
			// <http://en.wikipedia.org/wiki/Native_POSIX_Thread_Library>
			// <http://www.net.in.tum.de/~gregor/docs/pthread-scheduling.html>
			//
			// Manuals suggest checking #ifdef _POSIX_THREAD_PRIORITY_SCHEDULING,
			// but even Ubuntu does not seem to define it.
			//
#			define _PRIO_MODE SCHED_RR

			// NTLP 2.5: only system scope allowed (being the basic reason why
			//           root privileges are required..)
			//#define _PRIO_SCOPE PTHREAD_SCOPE_PROCESS

#			define _PRIO_HI 99
#			define _PRIO_0  50
#			define _PRIO_LO 1

#		elif defined(PLATFORM_BSD)
			//
			// <http://www.net.in.tum.de/~gregor/docs/pthread-scheduling.html>
			//
			// "When control over the thread scheduling is desired, then FreeBSD
			//  with the libpthread implementation is by far the best choice .."
			//
#			define _PRIO_MODE SCHED_OTHER
#			define _PRIO_SCOPE PTHREAD_SCOPE_PROCESS
#			define _PRIO_HI 31
#			define _PRIO_0  15
#			define _PRIO_LO 1

#		elif defined(PLATFORM_CYGWIN)
			//
			// TBD: Find right values for Cygwin
			//
#		elif defined( PLATFORM_WIN32) || defined( PLATFORM_POCKETPC)
			// any other value not supported by win32-pthread as of version 2.9.1
#			define _PRIO_MODE SCHED_OTHER

			// PTHREAD_SCOPE_PROCESS not supported by win32-pthread as of version 2.9.1
			//#define _PRIO_SCOPE PTHREAD_SCOPE_SYSTEM // but do we need this at all to start with?
			THREAD_PRIORITY_IDLE, THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_BELOW_NORMAL, THREAD_PRIORITY_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL, THREAD_PRIORITY_HIGHEST, THREAD_PRIORITY_TIME_CRITICAL

#		else
#			error "Unknown OS: not implemented!"
#		endif

#if defined _PRIO_0
#	define _PRIO_AN (_PRIO_0 + ((_PRIO_HI-_PRIO_0)/2))
#	define _PRIO_BN (_PRIO_LO + ((_PRIO_0-_PRIO_LO)/2))

	_PRIO_LO, _PRIO_LO, _PRIO_BN, _PRIO_0, _PRIO_AN, _PRIO_HI, _PRIO_HI
#endif // _PRIO_0
};

void THREAD_CREATE( THREAD_T* ref, THREAD_RETURN_T (*func)( void*), void* data, int prio /* -3..+3 */)
{
	pthread_attr_t a;
	bool_t const normal =
#if defined(PLATFORM_LINUX) && defined(LINUX_SCHED_RR)
	!sudo; // with sudo, even normal thread must use SCHED_RR
#else
	(prio == 0);
#endif

	PT_CALL( pthread_attr_init( &a));

#ifndef PTHREAD_TIMEDJOIN
	// We create a NON-JOINABLE thread. This is mainly due to the lack of
	// 'pthread_timedjoin()', but does offer other benefits (s.a. earlier 
	// freeing of the thread's resources).
	//
	PT_CALL( pthread_attr_setdetachstate( &a, PTHREAD_CREATE_DETACHED));
#endif // PTHREAD_TIMEDJOIN

	// Use this to find a system's default stack size (DEBUG)
#if 0
	{
		size_t n;
		pthread_attr_getstacksize( &a, &n);
		fprintf( stderr, "Getstack: %u\n", (unsigned int)n);
	}
	//  524288 on OS X
	// 2097152 on Linux x86 (Ubuntu 7.04)
	// 1048576 on FreeBSD 6.2 SMP i386
#endif // 0

#if defined _THREAD_STACK_SIZE && _THREAD_STACK_SIZE > 0
	PT_CALL( pthread_attr_setstacksize( &a, _THREAD_STACK_SIZE));
#endif

	if( !normal)
	{
		struct sched_param sp;
		// "The specified scheduling parameters are only used if the scheduling
		//  parameter inheritance attribute is PTHREAD_EXPLICIT_SCHED."
		//
		PT_CALL( pthread_attr_setinheritsched( &a, PTHREAD_EXPLICIT_SCHED));

#ifdef _PRIO_SCOPE
		PT_CALL( pthread_attr_setscope( &a, _PRIO_SCOPE));
#endif // _PRIO_SCOPE

		PT_CALL( pthread_attr_setschedpolicy( &a, _PRIO_MODE));

		// prio range [-3,+3] was checked by the caller
		sp.sched_priority = gs_prio_remap[ prio + 3];
		PT_CALL( pthread_attr_setschedparam( &a, &sp));
	}

	//---
	// Seems on OS X, _POSIX_THREAD_THREADS_MAX is some kind of system
	// thread limit (not userland thread). Actual limit for us is way higher.
	// PTHREAD_THREADS_MAX is not defined (even though man page refers to it!)
	//
# ifndef THREAD_CREATE_RETRIES_MAX
	// Don't bother with retries; a failure is a failure
	//
	{ 
		int rc = pthread_create( ref, &a, func, data);
		if( rc) _PT_FAIL( rc, "pthread_create()", __FILE__, __LINE__ - 1);
	}
# else
#		error "This code deprecated"
		/*
		// Wait slightly if thread creation has exchausted the system
		//
		{ uint_t retries;
		for( retries=0; retries<THREAD_CREATE_RETRIES_MAX; retries++ ) {

		int rc= pthread_create( ref, &a, func, data );
		//
		// OS X / Linux:
		//    EAGAIN: ".. lacked the necessary resources to create
		//             another thread, or the system-imposed limit on the
		//             total number of threads in a process 
		//             [PTHREAD_THREADS_MAX] would be exceeded."
		//    EINVAL: attr is invalid
		// Linux:
		//    EPERM: no rights for given parameters or scheduling (no sudo)
		//    ENOMEM: (known to fail with this code, too - not listed in man)

		if (rc==0) break;   // ok!

		// In practise, exhaustion seems to be coming from memory, not a
		// maximum number of threads. Keep tuning... ;)
		//
		if (rc==EAGAIN) {
		//fprintf( stderr, "Looping (retries=%d) ", retries );    // DEBUG

		// Try again, later.

		Yield();
		} else {
		_PT_FAIL( rc, "pthread_create()", __FILE__, __LINE__ );
		}
		}
		}
		*/
# endif

	PT_CALL( pthread_attr_destroy( &a));
}


void THREAD_SET_PRIORITY( int prio)
{
#if defined PLATFORM_LINUX && defined LINUX_SCHED_RR
	if( sudo) // only root-privileged process can change priorities
#endif // defined PLATFORM_LINUX && defined LINUX_SCHED_RR
	{
		struct sched_param sp;
		// prio range [-3,+3] was checked by the caller
		sp.sched_priority = gs_prio_remap[ prio + 3];
		PT_CALL( pthread_setschedparam( pthread_self(), _PRIO_MODE, &sp));
	}
}


 /*
  * Wait for a thread to finish.
  *
  * 'mu_ref' is a lock we should use for the waiting; initially unlocked.
  * Same lock as passed to THREAD_EXIT.
  *
  * Returns TRUE for successful wait, FALSE for timed out
  */
bool_t THREAD_WAIT( THREAD_T *ref, double secs , SIGNAL_T *signal_ref, MUTEX_T *mu_ref, volatile enum e_status *st_ref)
{
    struct timespec ts_store;
    const struct timespec *timeout= NULL;
    bool_t done;

    // Do timeout counting before the locks
    //
#if THREADWAIT_METHOD == THREADWAIT_TIMEOUT
    if (secs>=0.0)
#else // THREADWAIT_METHOD == THREADWAIT_CONDVAR
    if (secs>0.0)
#endif // THREADWAIT_METHOD == THREADWAIT_CONDVAR
    {
        prepare_timeout( &ts_store, now_secs()+secs );
        timeout= &ts_store;
    }

#if THREADWAIT_METHOD == THREADWAIT_TIMEOUT
    /* Thread is joinable
    */
    if (!timeout) {
        PT_CALL( pthread_join( *ref, NULL /*ignore exit value*/ ));
        done= TRUE;
    } else {
        int rc= PTHREAD_TIMEDJOIN( *ref, NULL, timeout );
        if ((rc!=0) && (rc!=ETIMEDOUT)) {
            _PT_FAIL( rc, "PTHREAD_TIMEDJOIN", __FILE__, __LINE__-2 );
        }
        done= rc==0;
    }
#else // THREADWAIT_METHOD == THREADWAIT_CONDVAR
    /* Since we've set the thread up as PTHREAD_CREATE_DETACHED, we cannot
     * join with it. Use the cond.var.
    */
    (void) ref; // unused
    MUTEX_LOCK( mu_ref );
    
        // 'secs'==0.0 does not need to wait, just take the current status
        // within the 'mu_ref' locks
        //
        if (secs != 0.0) {
            while( *st_ref < DONE ) {
                if (!timeout) {
                    PT_CALL( pthread_cond_wait( signal_ref, mu_ref ));
                } else {
                    int rc= pthread_cond_timedwait( signal_ref, mu_ref, timeout );
                    if (rc==ETIMEDOUT) break;
                    if (rc!=0) _PT_FAIL( rc, "pthread_cond_timedwait", __FILE__, __LINE__-2 );
                }
            }
        }
        done= *st_ref >= DONE;  // DONE|ERROR_ST|CANCELLED

    MUTEX_UNLOCK( mu_ref );
#endif // THREADWAIT_METHOD == THREADWAIT_CONDVAR
    return done;
  }
	//
  void THREAD_KILL( THREAD_T *ref ) {
    pthread_cancel( *ref );
  }

	void THREAD_MAKE_ASYNCH_CANCELLABLE()
	{
		// that's the default, but just in case...
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		// we want cancellation to take effect immediately if possible, instead of waiting for a cancellation point (which is the default)
		pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	}

	void THREAD_SETNAME( char const* _name)
	{
		// exact API to set the thread name is platform-dependant
		// if you need to fix the build, or if you know how to fill a hole, tell me (bnt.germain@gmail.com) so that I can submit the fix in github.
#if defined PLATFORM_BSD
		pthread_set_name_np( pthread_self(), _name);
#elif defined PLATFORM_LINUX
	#if LINUX_USE_PTHREAD_SETNAME_NP
		pthread_setname_np( pthread_self(), _name);
	#else // LINUX_USE_PTHREAD_SETNAME_NP
		 prctl(PR_SET_NAME, _name, 0, 0, 0);
	#endif // LINUX_USE_PTHREAD_SETNAME_NP
#elif defined PLATFORM_QNX || defined PLATFORM_CYGWIN
		pthread_setname_np( pthread_self(), _name);
#elif defined PLATFORM_OSX
		pthread_setname_np(_name);
#elif defined PLATFORM_WIN32 || defined PLATFORM_POCKETPC
		// no API in win32-pthread yet :-(
#endif
	}
#endif // THREADAPI == THREADAPI_PTHREAD

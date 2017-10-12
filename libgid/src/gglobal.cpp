#include <gglobal.h>
#include <time.h>

static g_id s_nextgid = 1;

#ifdef __cplusplus
extern "C" {
#endif

g_id g_NextId()
{
    return s_nextgid++;
}

#if defined(WINSTORE)
#elif defined(_WIN32)
#include <Windows.h>
#endif

#if defined(_WIN32) || defined(WINSTORE)
double g_iclock()
{
	static LARGE_INTEGER freq;
	static LARGE_INTEGER start;
	static bool init = false;

	if (init == false)
	{
		QueryPerformanceFrequency(&freq);
		QueryPerformanceCounter(&start);
		init = true;
	}

	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);

	LONGLONG delta = li.QuadPart - start.QuadPart;

	long double result = (long double)delta / (long double)freq.QuadPart;

	return (double)result;
}
#elif defined(__APPLE__)
#import <CoreFoundation/CoreFoundation.h>
double g_iclock()
{
	static double begin = CFAbsoluteTimeGetCurrent();
	return CFAbsoluteTimeGetCurrent() - begin;
}
#elif defined(__ANDROID__) || defined(STRICT_LINUX)
static double nanoTime()
{
	struct timespec t;
	if(clock_gettime(CLOCK_MONOTONIC, &t))
		return 0;
	return t.tv_sec + t.tv_nsec * 1e-9;
}
double g_iclock()
{
	static double begin = nanoTime();
	return nanoTime() - begin;
}
#elif __EMSCRIPTEN__
#include "emscripten.h"
double g_iclock()
{
	return emscripten_get_now()/1000;
}
#else
double g_iclock()
{
	double CPS = CLOCKS_PER_SEC;

	clock_t clock = ::clock();

	static clock_t lastClock;
	static bool firstCall = true;
	static double result = 0;

	if (firstCall)
	{
		lastClock = clock;
		firstCall = false;
	}

	clock_t delta = clock - lastClock;
	result += delta / CPS;
	lastClock = clock;

	return result;
}
#endif

#ifdef __cplusplus
}
#endif

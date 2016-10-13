#ifndef PLATFORM_H_INCLUDED
#define PLATFORM_H_INCLUDED

#include <math.h>
#include "platformutil.h"

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

//#if defined(__GNUC__) && ( defined(__APPLE_CPP__) || defined(__APPLE_CC__) || defined(__MACOS_CLASSIC__) )
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#define __G_TARGET_IPHONE__
#endif

/*

#ifdef __APPLE__
#import <CoreFoundation/CoreFoundation.h>
inline double iclock()
{
	static double begin = -1;
	if (begin < 0)
		begin = CFAbsoluteTimeGetCurrent();

	return CFAbsoluteTimeGetCurrent() - begin;
}
#else
inline double iclock()
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

*/


#ifdef __G_TARGET_IPHONE__

#include "CoreFoundation/CoreFoundation.h"
#import <QuartzCore/QuartzCore.h>
//#include "ApplicationServices/ApplicationServices.h"

inline char* getFileName(const char* filename)
{
	char* file = (char *)malloc(strlen(filename) + 1);
	strcpy(file, filename);
	
	char* ext = strrchr(file, '.') + 1;
	*(ext - 1) = 0;
	
    CFStringRef name = CFStringCreateWithBytes(NULL, (UInt8*)file, strlen(file), kCFStringEncodingUTF8, false);
    CFStringRef type = CFStringCreateWithBytes(NULL, (UInt8*)ext, strlen(ext), kCFStringEncodingUTF8, false);
	
	free(file);
	
	CFURLRef url = CFBundleCopyResourceURL(CFBundleGetMainBundle(), name, type, NULL);
	
	CFStringRef string = CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
	
	CFIndex length = ::CFStringGetLength(string);
	CFIndex bufLen = 0;
	::CFStringGetBytes(string, CFRangeMake(0, length), kCFStringEncodingUTF8,
					   0, false, NULL, 0, &bufLen);
	
	UInt8* buffer = (UInt8*)malloc(bufLen + 1);

	::CFStringGetBytes(string, CFRangeMake(0, length), kCFStringEncodingUTF8,
					   0, false, buffer, bufLen, NULL);
	buffer[bufLen] = 0;
	
	CFRelease(string);
	CFRelease(url);
	
	return (char*)buffer;
}


/*
inline void DebugPrint(char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}
*/

#define _snprintf snprintf

#else

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#include <Windows.h>
#include <malloc.h>
#endif
#include <time.h>
/*
// TODO: now this is obsolute, use pathForFile
inline char* getFileName(const char* filename)
{
	char* buffer = (char*)malloc(strlen(filename) + 1);
	strcpy(buffer, filename);

	return buffer;
}

inline void DebugPrint(const char *fmt, ...)
{
	va_list ap;

	if (fmt == NULL)
		return;

	va_start(ap, fmt);
	int size = _vsnprintf(NULL, 0, fmt, ap);
	char* text = (char*)alloca(size + 1);
	vsprintf(text, fmt, ap);
	va_end(ap);

	OutputDebugStringA(text);
}
*/


#endif

/*
// accelerometer
void setAccelerometer(double x, double y, double z);
void getAccelerometer(double* x, double* y, double* z);
*/

//const int hardwareWidth = 320;
//const int hardwareHeight = 480;

const double M_NAN = log(-1.0);
const float F_NAN = log(-1.f);

void openUrl(const char* url);
bool canOpenUrl(const char* url);
/*
void memoryWarning();						// bunun burada olmasi sacma olmus

class PlatformInterface
{
public:
	virtual ~PlatformInterface() {}

	virtual void openUrl(const char* url) = 0;
	virtual void memoryWarning() = 0;		// bunun burada olmasi sacma olmus
};

void setPlatformInterface(PlatformInterface* platformInterface);
*/

void setWindowSize(int width, int height);
void setFullScreen(bool fullScreen);
std::string getDeviceName();

void vibrate(int ms);
std::string getLocale();
std::string getLanguage();
void setKeepAwake(bool awake);
bool setKeyboardVisibility(bool visible);
std::vector<std::string> getDeviceInfo();

bool g_checkStringProperty(bool isSet, const char* what);
const char* g_getProperty(const char* what, const char* arg);
void g_setProperty(const char* what, const char* arg);

#endif

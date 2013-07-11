#include <QtCore>
#include "uid.h"

#if defined(Q_OS_WIN)
#include <windows.h>
std::string g_uid()
{
	std::string result;

	HKEY hKey = 0;
	if (RegOpenKeyA(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Cryptography", &hKey) == ERROR_SUCCESS)
	{
		char buf[256] = {0};
		DWORD bufSize = sizeof(buf);

		if (RegQueryValueExA(hKey, "MachineGuid", 0, 0, (LPBYTE)buf, &bufSize) == ERROR_SUCCESS)
			result = buf;

		RegCloseKey(hKey);
	}

	return result;
}
#elif defined(Q_OS_MAC)

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>

static std::string CFStringToSTLString(const CFStringRef cfstr)
{
	// you could pass in a different encoding if desired.

	// i'm not sure if CFStringGetMaximumSizeForEncoding
	// includes space for the termination character or not?
	// so i add 1 here to make sure..
	CFIndex buf_len = 1 + CFStringGetMaximumSizeForEncoding(
					CFStringGetLength(cfstr),
					kCFStringEncodingUTF8);
	char *buffer = new char[buf_len];
	Boolean bool2 = CFStringGetCString(cfstr, buffer, buf_len, kCFStringEncodingUTF8);

	std::string myString(buffer);
	delete[] buffer;

	return myString;
}

std::string g_uid()
{
	io_registry_entry_t ioRegistryRoot = IORegistryEntryFromPath(kIOMasterPortDefault, "IOService:/");
	CFStringRef uuidCf = (CFStringRef) IORegistryEntryCreateCFProperty(ioRegistryRoot, CFSTR(kIOPlatformUUIDKey), kCFAllocatorDefault, 0);
	IOObjectRelease(ioRegistryRoot);
	std::string result = CFStringToSTLString(uuidCf);
	CFRelease(uuidCf);

	return result;
}
#endif


#if 0
QSet<QString> macs;
foreach(QNetworkInterface interface, QNetworkInterface::allInterfaces())
{
	if (  interface.isValid() &&
		!(interface.flags() & QNetworkInterface::IsLoopBack) &&
		 (interface.flags() & QNetworkInterface::IsUp) &&
		 (interface.flags() & QNetworkInterface::IsRunning))
	{
		QString mac = interface.hardwareAddress();
		QString name = interface.humanReadableName();
		if (!mac.isEmpty() &&
			 mac.length() == 17 &&
			!name.startsWith("Local Area Connection*"))
		{
			macs.insert(mac);
		}
	}
}
#endif

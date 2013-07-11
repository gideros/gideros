#include <vector>
#include <string>

#include <QObject>

std::vector<std::string> getDeviceInfo()
{
	std::vector<std::string> result;

#ifdef Q_OS_WIN
	result.push_back("Windows");
#endif
#ifdef Q_OS_MAC
	result.push_back("Mac OS");
#endif

	return result;
}

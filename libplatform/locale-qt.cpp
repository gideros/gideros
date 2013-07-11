#include <string>
#include <QString>
#include <QStringList>
#include <QLocale>


std::string getLocale()
{
	return QLocale().name().toStdString();
}

std::string getLanguage()
{
	QString locale = QLocale().name();

	QStringList list = locale.split("_");

	if (!list.empty())
		return list[0].toStdString();

	return "en";
}

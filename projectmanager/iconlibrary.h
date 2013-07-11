#ifndef ICONLIBRARY_H
#define ICONLIBRARY_H

#include <QIcon>
#include <QString>
#include <map>

class IconLibrary
{
public:
	static IconLibrary& instance();
	const QIcon& icon(const QString& name) const;

private:
	IconLibrary();
	~IconLibrary();

	std::map<QString, QIcon> iconMap_;
	QImage image_;

	QIcon icon(int i, int j) const;
	QIcon icon(int i0, int j0, int i1, int j1, int dx, int dy) const;
};

#endif // ICONLIBRARY_H

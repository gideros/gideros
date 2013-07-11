#ifndef LICENSEMANAGER_H
#define LICENSEMANAGER_H

#include <QObject>
#include <QNetworkProxy>


extern QNetworkProxy g_proxy;
extern bool g_useProxy;
extern void g_querySystemProxy();

extern void g_updateHash();
extern bool g_checkHash();

class QNetworkAccessManager;
class QNetworkReply;

class LicenseManager : public QObject
{
	Q_OBJECT

public:
	LicenseManager();
	virtual ~LicenseManager();

	void updateLicense();
	void checkExpired();
	int getLicenseType() const;

private slots:
	void updateLicenseFinished(QNetworkReply*);

private:
	QNetworkAccessManager* updateLicenseManager_;
};


#endif

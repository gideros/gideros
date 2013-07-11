#include "licensemanager.h"
#include "uid.h"
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>
#include <QDomDocument>
#include <QDate>
#include <QDebug>
#include <QStringList>
#include <QNetworkProxy>
#include <QNetworkProxyQuery>
#include <QUrl>
#include <QDebug>
#include <QCryptographicHash>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif

QNetworkProxy g_proxy;
bool g_useProxy = false;

void g_querySystemProxy()
{
	QNetworkProxyQuery npq(QUrl("http://giderosmobile.com"));
	QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(npq);

	for (int i = 0; i < proxies.size(); ++i)
	{
		if (proxies[i].type() != QNetworkProxy::NoProxy)
		{
			g_proxy = proxies[i];
			g_useProxy = true;
			break;
		}
	}
}

LicenseManager::LicenseManager()
{
	updateLicenseManager_ = new QNetworkAccessManager(this);
	if (g_useProxy)
		updateLicenseManager_->setProxy(g_proxy);
	connect(updateLicenseManager_, SIGNAL(finished(QNetworkReply*)), this, SLOT(updateLicenseFinished(QNetworkReply*)));
}

LicenseManager::~LicenseManager()
{
	delete updateLicenseManager_;
}

void LicenseManager::updateLicense()
{
	QUrl url("http://giderosmobile.com/g-userinfo.php");
#ifdef Q_OS_MAC
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "giderosmobile.com", "GiderosLicenseManager");
#else
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "GiderosMobile", "GiderosLicenseManager");
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QUrlQuery q;
    q.addQueryItem("login", settings.value("login").toString());
    q.addQueryItem("uid", g_uid().c_str());
    url.setQuery(q);
#else
    url.addQueryItem("login", settings.value("login").toString());
    url.addQueryItem("uid", g_uid().c_str());
#endif
	QNetworkRequest request(url);
	updateLicenseManager_->get(request);
}

void LicenseManager::updateLicenseFinished(QNetworkReply* reply)
{
	if (reply->error() != QNetworkReply::NoError
		// a web page that returns 403/404 can still have content
		&& reply->error() != QNetworkReply::ContentOperationNotPermittedError
		&& reply->error() != QNetworkReply::ContentNotFoundError)

	{
		reply->deleteLater();
		return;
	}

	QString str = reply->readAll();
	reply->deleteLater();

	QDomDocument doc;
	if (doc.setContent(str) == true)
	{
		QDomElement root = doc.documentElement();
		if (root.tagName() == "user")
		{
#ifdef Q_OS_MAC
			QSettings settings(QSettings::IniFormat, QSettings::UserScope, "giderosmobile.com", "GiderosLicenseManager");
#else
			QSettings settings(QSettings::IniFormat, QSettings::UserScope, "GiderosMobile", "GiderosLicenseManager");
#endif
			settings.setValue("type", root.attribute("type").toInt());
			settings.setValue("expire", root.attribute("expire"));
			settings.setValue("date", root.attribute("date"));
			settings.setValue("expired", false);
			settings.sync();
			g_updateHash();
			checkExpired();
		}
		else if (root.tagName() == "error")
		{
#ifdef Q_OS_MAC
			QSettings settings(QSettings::IniFormat, QSettings::UserScope, "giderosmobile.com", "GiderosLicenseManager");
#else
			QSettings settings(QSettings::IniFormat, QSettings::UserScope, "GiderosMobile", "GiderosLicenseManager");
#endif
			settings.setValue("type", -1);
			settings.sync();
			g_updateHash();
		}
	}
}


void LicenseManager::checkExpired()
{
#ifdef Q_OS_MAC
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "giderosmobile.com", "GiderosLicenseManager");
#else
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "GiderosMobile", "GiderosLicenseManager");
#endif

	int type = settings.value("type", -1).toInt();

	if (type == 2 || type == 3)
	{
		QDate expire = QDate::fromString(settings.value("expire").toString(), "yyyy-MM-dd");
		QDate date = QDate::fromString(settings.value("date").toString(), "yyyy-MM-dd");

		if (expire < date || expire < QDate::currentDate())
		{
			settings.setValue("expired", true);
			settings.sync();
		}
	}
}


int LicenseManager::getLicenseType() const
{
#ifdef Q_OS_MAC
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "giderosmobile.com", "GiderosLicenseManager");
#else
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "GiderosMobile", "GiderosLicenseManager");
#endif

	return settings.value("type", -1).toInt();
}

void g_updateHash()
{
#ifdef Q_OS_MAC
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "giderosmobile.com", "GiderosLicenseManager");
#else
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "GiderosMobile", "GiderosLicenseManager");
#endif

	QByteArray data;
	data.append("salt");
	data.append(settings.value("login").toString());
	data.append(settings.value("type", -1).toInt());
	data.append(settings.value("expire").toString());
	data.append("pepper");

	QString hash = QString(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex());

	settings.setValue("hash", hash);

	settings.sync();
}


bool g_checkHash()
{
#ifdef Q_OS_MAC
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "giderosmobile.com", "GiderosLicenseManager");
#else
	QSettings settings(QSettings::IniFormat, QSettings::UserScope, "GiderosMobile", "GiderosLicenseManager");
#endif

	QByteArray data;
	data.append("salt");
	data.append(settings.value("login").toString());
	data.append(settings.value("type", -1).toInt());
	data.append(settings.value("expire").toString());
	data.append("pepper");

	QString hash1 = QString(QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex());

	QString hash2 = settings.value("hash").toString();

	return hash1 == hash2;
}

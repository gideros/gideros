#include "deactivatewidget.h"
#include "ui_deactivatewidget.h"
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QNetworkReply>
#include <QDebug>
#include <uid.h>
#include <QMessageBox>
#include <QDomDocument>
#include <licensemanager.h>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif


DeactivateWidget::DeactivateWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeactivateWidget)
{
    ui->setupUi(this);

	updateLicenseManager_ = new QNetworkAccessManager(this);
	if (g_useProxy)
		updateLicenseManager_->setProxy(g_proxy);
	deactivateManager_ = new QNetworkAccessManager(this);
	if (g_useProxy)
		deactivateManager_->setProxy(g_proxy);

	connect(updateLicenseManager_, SIGNAL(finished(QNetworkReply*)), this, SLOT(updateLicenseFinished(QNetworkReply*)));
	connect(deactivateManager_, SIGNAL(finished(QNetworkReply*)), this, SLOT(deactivateFinished(QNetworkReply*)));
}

DeactivateWidget::~DeactivateWidget()
{
	delete deactivateManager_;
	delete updateLicenseManager_;
    delete ui;
}

void DeactivateWidget::disableUI()
{
	ui->updateLicense->setEnabled(false);
	ui->deactivate->setEnabled(false);
}

void DeactivateWidget::enableUI()
{
	ui->updateLicense->setEnabled(true);
	ui->deactivate->setEnabled(true);
}

void DeactivateWidget::on_updateLicense_clicked()
{
	disableUI();
	QUrl url("http://giderosmobile.com/g-userinfo.php");
	QSettings settings;
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

void DeactivateWidget::on_deactivate_clicked()
{
	disableUI();

	QUrl url("http://giderosmobile.com/g-deactivate.php");
	QSettings settings;
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
	deactivateManager_->get(request);
}

void DeactivateWidget::updateLicenseFinished(QNetworkReply* reply)
{
	enableUI();

	if (reply->error() != QNetworkReply::NoError
		// a web page that returns 403/404 can still have content
		&& reply->error() != QNetworkReply::ContentOperationNotPermittedError
		&& reply->error() != QNetworkReply::ContentNotFoundError)

	{
		QMessageBox::warning(this, "Error", QString("Failed to connect to the server. Error code: %1").arg(reply->error()));
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
			QSettings settings;
			settings.setValue("type", root.attribute("type").toInt());
			settings.setValue("expire", root.attribute("expire"));
			settings.setValue("date", root.attribute("date"));
			settings.setValue("expired", false);
			settings.sync();
			g_updateHash();
			emit updateUI();
		}
		else if (root.tagName() == "error")
		{
			QMessageBox::warning(this, "Error", root.text());
		}
		else
		{
			QMessageBox::warning(this, "Error", "Unknown server response.");
		}
	}
	else
	{
		QMessageBox::warning(this, "Error", "Failed to parse the server response.");
	}
}

void DeactivateWidget::deactivateFinished(QNetworkReply* reply)
{
	enableUI();

	if (reply->error() != QNetworkReply::NoError
		// a web page that returns 403/404 can still have content
		&& reply->error() != QNetworkReply::ContentOperationNotPermittedError
		&& reply->error() != QNetworkReply::ContentNotFoundError)

	{
		askUser(QString("Failed to connect to the server. Error code: %1").arg(reply->error()));
		reply->deleteLater();
		return;
	}

	QString str = reply->readAll();
	reply->deleteLater();

	QDomDocument doc;
	if (doc.setContent(str) == true)
	{
		QDomElement root = doc.documentElement();

		if (root.tagName() == "ok")
		{
			QSettings settings;
			settings.setValue("type", -1);
			settings.sync();
			g_updateHash();
			emit updateUI();
		}
		else if (root.tagName() == "error")
		{
			askUser(root.text());
		}
		else
		{
			askUser("Unknown server response.");
		}
	}
	else
	{
		askUser("Failed to parse the server response.");
	}
}


void DeactivateWidget::askUser(const QString& message)
{
	int result = QMessageBox::warning(this, "Error", message + "\nDo you still want to deauthorize this computer?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (result == QMessageBox::Yes)
	{
		QSettings settings;
		settings.setValue("type", -1);
		settings.sync();
		g_updateHash();
		emit updateUI();
	}
}

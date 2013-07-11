#include "deactivatealldialog.h"
#include "ui_deactivatealldialog.h"
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMessageBox>
#include <QDomDocument>
#include <licensemanager.h>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif

DeactivateAllDialog::DeactivateAllDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeactivateAllDialog)
{
    ui->setupUi(this);

	QSettings settings;
	ui->login->setText(settings.value("login").toString());

	manager_ = new QNetworkAccessManager(this);
	if (g_useProxy)
		manager_->setProxy(g_proxy);
	connect(manager_, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}

DeactivateAllDialog::~DeactivateAllDialog()
{
	delete manager_;
    delete ui;
}

void DeactivateAllDialog::on_deactivateAll_clicked()
{
	ui->deactivateAll->setEnabled(false);

	QUrl url("http://giderosmobile.com/g-deactivateall.php");
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QUrlQuery q;
    q.addQueryItem("login", ui->login->text());
    q.addQueryItem("password", ui->password->text());
    url.setQuery(q);
#else
	url.addQueryItem("login", ui->login->text());
	url.addQueryItem("password", ui->password->text());
#endif
	QNetworkRequest request(url);
	manager_->get(request);
}

void DeactivateAllDialog::finished(QNetworkReply* reply)
{
	ui->deactivateAll->setEnabled(true);

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

		if (root.tagName() == "ok")
		{
			QSettings settings;
			settings.setValue("type", -1);
			settings.sync();
			g_updateHash();
			close();
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

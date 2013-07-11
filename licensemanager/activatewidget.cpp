#include "activatewidget.h"
#include "ui_activatewidget.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDomDocument>
#include <QSettings>
#include <QMessageBox>
#include <uid.h>
#include <licensemanager.h>

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QUrlQuery>
#endif

ActivateWidget::ActivateWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActivateWidget)
{
    ui->setupUi(this);

	QSettings settings;
	ui->login->setText(settings.value("login").toString());
	//ui->password->setText(settings.value("password").toString());

	manager_ = new QNetworkAccessManager(this);
	if (g_useProxy)
		manager_->setProxy(g_proxy);
	connect(manager_, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished(QNetworkReply*)));
}

ActivateWidget::~ActivateWidget()
{
	delete manager_;
    delete ui;
}

void ActivateWidget::on_loginButton_clicked()
{
	ui->loginButton->setEnabled(false);

	QUrl url("http://giderosmobile.com/g-activate.php");
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QUrlQuery q;
    q.addQueryItem("login", ui->login->text());
    q.addQueryItem("password", ui->password->text());
    q.addQueryItem("uid", g_uid().c_str());
    url.setQuery(q);
#else
	url.addQueryItem("login", ui->login->text());
	url.addQueryItem("password", ui->password->text());
	url.addQueryItem("uid", g_uid().c_str());
#endif
	QNetworkRequest request(url);
	manager_->get(request);
}

void ActivateWidget::finished(QNetworkReply* reply)
{
	ui->loginButton->setEnabled(true);

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
			settings.setValue("login", ui->login->text());
			//settings.setValue("password", ui->password->text());
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

void ActivateWidget::on_login_returnPressed()
{
	on_loginButton_clicked();
}

void ActivateWidget::on_password_returnPressed()
{
	on_loginButton_clicked();
}


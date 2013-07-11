#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "activatewidget.h"
#include "deactivatewidget.h"
#include <QSettings>
#include <QDebug>
#include "deactivatealldialog.h"
#include <QDate>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	activateWidget_ = new ActivateWidget(this);
	deactivateWidget_ = new DeactivateWidget(this);
	activateWidget_->hide();
	deactivateWidget_->hide();

	updateUI();

	connect(activateWidget_, SIGNAL(updateUI()), this, SLOT(updateUI()));
	connect(deactivateWidget_, SIGNAL(updateUI()), this, SLOT(updateUI()));
	connect(ui->actionDeactivate_All, SIGNAL(triggered()), this, SLOT(deactivateAll()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateUI()
{
	checkExpired();

	QSettings settings;

	int type = settings.value("type", -1).toInt();

	if (activateWidget_->isHidden() && deactivateWidget_->isHidden())
	{
		if (type == -1)
		{
			activateWidget_->show();
			centralWidget()->layout()->addWidget(activateWidget_);
		}
		else
		{
			deactivateWidget_->show();
			centralWidget()->layout()->addWidget(deactivateWidget_);
		}
	}
	else if (activateWidget_->isVisible() && type != -1)
	{
		activateWidget_->hide();
		centralWidget()->layout()->removeWidget(activateWidget_);

		deactivateWidget_->show();
		centralWidget()->layout()->addWidget(deactivateWidget_);
	}
	else if (deactivateWidget_->isVisible() && type == -1)
	{
		deactivateWidget_->hide();
		centralWidget()->layout()->removeWidget(deactivateWidget_);

		activateWidget_->show();
		centralWidget()->layout()->addWidget(activateWidget_);
	}

	if (type == -1)
	{
		ui->licenseInfo->setHtml(
		"<html>"
		"<body>"
		"<p>You do not have a license installed. Please login to get the license info.</p>"
		"<p>If you do not have an account, you can register from <a href=\"http://www.giderosmobile.com\">www.giderosmobile.com</a>.</p>"
		"</body>"
		"</html>"
		);
	}
	else if (type == 1)
	{
		ui->licenseInfo->setHtml(
		"<html>"
		"<body>"
		"<p>You have a free license. Your application will show a splash screen when run on the device.</p>"
		"<p>You can purchase an Indie or Professional license from <a href=\"http://www.giderosmobile.com\">www.giderosmobile.com</a>.</p>"
		"</body>"
		"</html>"
		);
	}
	else if (type == 2)
	{
		bool expired = settings.value("expired").toBool();

		if (expired)
		{
			ui->licenseInfo->setHtml(QString(
			"<html>"
			"<body>"
			"<p>You have an indie developer license, valid for indie game developers or companies.</p>"
			"<p><span style=\"color:#ff0000;\">Your subscription has expired on %1.</span></p>"
			"</body>"
			"</html>"
			).arg(settings.value("expire").toString()));
		}
		else
		{
			ui->licenseInfo->setHtml(QString(
			"<html>"
			"<body>"
			"<p>You have an indie developer license, valid for indie game developers or companies.</p>"
			"<p>Your subscription will expire on %1.</p>"
			"</body>"
			"</html>"
			).arg(settings.value("expire").toString()));
		}
	}
	else if (type == 3)
	{
		bool expired = settings.value("expired").toBool();

		if (expired)
		{
			ui->licenseInfo->setHtml(QString(
			"<html>"
			"<body>"
			"<p>You have professional developer license, valid for enterprise companies.</p>"
			"<p><span style=\"color:#ff0000;\">Your subscription has expired on %1.</span></p>"
			"</body>"
			"</html>"
			).arg(settings.value("expire").toString()));
		}
		else
		{
			ui->licenseInfo->setHtml(QString(
			"<html>"
			"<body>"
			"<p>You have professional developer license, valid for enterprise companies.</p>"
			"<p>Your subscription will expire on %1.</p>"
			"</body>"
			"</html>"
			).arg(settings.value("expire").toString()));
		}
	}
}

void MainWindow::deactivateAll()
{
	DeactivateAllDialog dialog(this);
	dialog.exec();
	updateUI();
}

void MainWindow::checkExpired()
{
	QSettings settings;

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

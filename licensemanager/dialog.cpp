#include "dialog.h"
#include "ui_dialog.h"
#include <QNetworkInterface>
#include <QStringList>
#include "activatewidget.h"
#include "deactivatewidget.h"
#include <QUuid>
#include <QSettings>
#include <QDebug>
#include "uid.h"

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

	activateWidget_ = new ActivateWidget(this);
	deactivateWidget_ = new DeactivateWidget(this);
	activateWidget_->hide();
	deactivateWidget_->hide();

	updateUI();

	connect(activateWidget_, SIGNAL(updateUI()), this, SLOT(updateUI()));
	connect(deactivateWidget_, SIGNAL(updateUI()), this, SLOT(updateUI()));

	qDebug() << g_uid().c_str();
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::updateUI()
{
	QSettings settings;

	int type = settings.value("type", -1).toInt();

	if (activateWidget_->isHidden() && deactivateWidget_->isHidden())
	{
		if (type == -1)
		{
			qDebug() << "ActivateWidget";
			activateWidget_->show();
			layout()->addWidget(activateWidget_);
		}
		else
		{
			qDebug() << "DeactivateWidget";
			deactivateWidget_->show();
			layout()->addWidget(deactivateWidget_);
		}
	}
	else if (activateWidget_->isVisible() && type != -1)
	{
		activateWidget_->hide();
		layout()->removeWidget(activateWidget_);

		qDebug() << "DeactivateWidget";
		deactivateWidget_->show();
		layout()->addWidget(deactivateWidget_);
	}
	else if (deactivateWidget_->isVisible() && type == -1)
	{
		deactivateWidget_->hide();
		layout()->removeWidget(deactivateWidget_);

		qDebug() << "ActivateWidget";
		activateWidget_->show();
		layout()->addWidget(activateWidget_);
	}
}


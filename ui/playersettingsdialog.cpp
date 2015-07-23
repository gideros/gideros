#include "playersettingsdialog.h"
#include <QSettings>

PlayerSettingsDialog::PlayerSettingsDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(ok()));
	connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));
	connect(ui.localhost, SIGNAL(toggled(bool)), this, SLOT(localhostToggled(bool)));

	QSettings settings;

	QString ip = settings.value("player ip", QString("127.0.0.1")).toString();
	originalip_ = settings.value("player original ip", ip).toString();
	ui.ip->setText(originalip_);

	bool localhost = settings.value("player localhost", true).toBool();
	ui.localhost->setChecked(localhost);
}

PlayerSettingsDialog::~PlayerSettingsDialog()
{

}

void PlayerSettingsDialog::ok()
{
	if (!ui.localhost->isChecked())
		originalip_ = ui.ip->text();

	QSettings settings;
	settings.setValue("player ip", ui.ip->text());
	settings.setValue("player original ip", originalip_);
	settings.setValue("player localhost", ui.localhost->isChecked());

	accept();
}

void PlayerSettingsDialog::cancel()
{
	reject();
}

void PlayerSettingsDialog::localhostToggled(bool checked)
{
	if (checked)
	{
		originalip_ = ui.ip->text();
		ui.ip->setText("127.0.0.1");
		ui.ip->setEnabled(false);
	}
	else
	{
		ui.ip->setText(originalip_);
		ui.ip->setEnabled(true);
	}
}

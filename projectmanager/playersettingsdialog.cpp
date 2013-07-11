#include "playersettingsdialog.h"
#include <QSettings>

PlayerSettingsDialog::PlayerSettingsDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QSettings settings;
	QString ip = settings.value("player ip", QString("127.0.0.1")).toString();
	ui.ip->setText(ip);

	connect(ui.ok, SIGNAL(clicked()), this, SLOT(ok()));
	connect(ui.cancel, SIGNAL(clicked()), this, SLOT(cancel()));
}

PlayerSettingsDialog::~PlayerSettingsDialog()
{

}

void PlayerSettingsDialog::ok()
{
	QSettings settings;
	settings.setValue("player ip", ip());

	accept();
}

void PlayerSettingsDialog::cancel()
{
	reject();
}

QString PlayerSettingsDialog::ip() const
{
	return ui.ip->text();
}

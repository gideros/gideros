#ifndef PLAYERSETTINGSDIALOG_H
#define PLAYERSETTINGSDIALOG_H

#include <QDialog>
#include "ui_playersettingsdialog.h"

class PlayerSettingsDialog : public QDialog
{
	Q_OBJECT

public:
	PlayerSettingsDialog(QWidget *parent = 0);
	~PlayerSettingsDialog();
	
private slots:
	void ok();
	void cancel();
	void localhostToggled(bool);

private:
	QString originalip_;

private:
	Ui::PlayerSettingsDialogClass ui;
};

#endif // PLAYERSETTINGSDIALOG_H

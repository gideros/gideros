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

	QString ip() const;
	
private slots:
	void ok();
	void cancel();

private:
	Ui::PlayerSettingsDialogClass ui;
};

#endif // PLAYERSETTINGSDIALOG_H

#ifndef SAVECHANGESDIALOG_H
#define SAVECHANGESDIALOG_H

#include <QDialog>
#include "ui_savechangesdialog.h"

class SaveChangesDialog : public QDialog
{
	Q_OBJECT

public:
	SaveChangesDialog(const QStringList& files, QWidget *parent = 0);
	~SaveChangesDialog();

private slots:
	void yes();
	void no();
	void cancel();

private:
	Ui::SaveChangesDialogClass ui;
};

#endif // SAVECHANGESDIALOG_H

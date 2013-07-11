#ifndef GOTOLINEDIALOG_H
#define GOTOLINEDIALOG_H

#include <QDialog>
#include "ui_gotolinedialog.h"

class GoToLineDialog : public QDialog
{
	Q_OBJECT

public:
	GoToLineDialog(QWidget *parent = 0);
	~GoToLineDialog();

	void setLineNumbers(int current, int max);
	int lineNumber() const;

private slots:
	void ok();
	void cancel();

private:
	Ui::GoToLineDialogClass ui;
	int lineNumber_;
};

#endif // GOTOLINEDIALOG_H

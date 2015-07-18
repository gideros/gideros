#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include "ui_finddialog.h"

class FindDialog : public QDialog
{
	Q_OBJECT

public:
	FindDialog(QWidget *parent = 0);
	~FindDialog();

	QString findWhat() const;
	bool wholeWord() const;
	bool matchCase() const;
	bool forward() const;

	void focusToFindWhat();

signals:
	void findNext();

private:
	Ui::FindDialogClass ui;
};

#endif // FINDDIALOG_H

#ifndef FINDINFILESDIALOG_H
#define FINDINFILESDIALOG_H

#include <QDialog>
#include "ui_findinfilesdialog.h"

class FindInFilesDialog : public QDialog
{
	Q_OBJECT
protected:
	void hideEvent(QHideEvent * event);

public:
	FindInFilesDialog(QWidget *parent = 0);
	~FindInFilesDialog();

	QString findWhat() const;
	QString filter() const;
	bool wholeWord() const;
	bool matchCase() const;
    bool regexp() const;

	void focusToFindWhat();

private:
	Ui::FindInFilesDialogClass ui;
};

#endif // FINDINFILESDIALOG_H

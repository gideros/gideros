#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>
#include "ui_replacedialog.h"

class ReplaceDialog : public QDialog
{
	Q_OBJECT

public:
	ReplaceDialog(QWidget *parent = 0);
	~ReplaceDialog();

	QString findWhat() const;
	QString replaceWith() const;
	bool wholeWord() const;
	bool matchCase() const;
	bool forward() const;

	void focusToFindWhat();

signals:
	void findNext();
	void replace();
	void replaceAll();

private:
	Ui::ReplaceDialogClass ui;
};

#endif // REPLACEDIALOG_H

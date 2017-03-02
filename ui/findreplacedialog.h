#ifndef FINDREPLACEDIALOG_H
#define FINDREPLACEDIALOG_H

#include <QDialog>
#include "ui_findreplacedialog.h"

class FindReplaceDialog : public QDialog
{
	Q_OBJECT
protected:
	void hideEvent(QHideEvent * event);

public:
	FindReplaceDialog(QWidget *parent = 0);
	~FindReplaceDialog();

	QString findWhat() const;
	bool wholeWord() const;
	bool matchCase() const;
	bool wrapAround() const;
	bool regularExpressions() const;

	void setMathcesText(const QString& str);

signals:
	void findNext();
	void findPrevious();

private:
	Ui::FindReplaceDialogClass ui;
};

#endif // FINDREPLACEDIALOG_H

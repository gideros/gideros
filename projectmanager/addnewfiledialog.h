#ifndef ADDNEWFILEDIALOG_H
#define ADDNEWFILEDIALOG_H

#include <QDialog>
#include "ui_addnewfiledialog.h"

class AddNewFileDialog : public QDialog
{
	Q_OBJECT

public:
	AddNewFileDialog(const QString& projectDirectory, QWidget *parent = 0);
	~AddNewFileDialog();

	QString fileName() const;
	QString location() const;

private slots:
	void on_browse_clicked();

private:
	Ui::AddNewFileDialogClass ui;

private:
	QString projectDirectory_;
};

#endif // ADDNEWFILEDIALOG_H

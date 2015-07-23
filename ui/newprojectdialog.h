#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>
#include "ui_newprojectdialog.h"
#include <QDir>

class NewProjectDialog : public QDialog
{
	Q_OBJECT

public:
	NewProjectDialog(QWidget *parent = 0);
	~NewProjectDialog();

	QString name() const;
	QString location() const;
	bool createDirectory() const;

	QString fullName() const;
	QString fullDirectory() const;

private:
	void updateWillCreateLabel();

private:
	Ui::NewProjectDialogClass ui;

private slots:
	void on_browse_clicked();
	void on_OK_clicked();
	void on_cancel_clicked();
	void on_name_textChanged(const QString& text);
	void on_location_textChanged(const QString& text);
	void on_createDirectory_stateChanged(int state);

};

#endif // NEWPROJECTDIALOG_H

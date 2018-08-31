#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include "ui_errordialog.h"

class ErrorDialog : public QDialog
{
	Q_OBJECT

public:
	ErrorDialog(QWidget *parent = 0);
	~ErrorDialog();

	void appendString(const QString& string);
public slots:
	void hide();
private:
	Ui::ErrorDialogClass ui;

};

#endif // ERRORDIALOG_H

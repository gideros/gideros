#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H

#include <QWidget>
#include "ui_optionswidget.h"

class OptionsWidget : public QWidget
{
	Q_OBJECT

public:
	OptionsWidget(QWidget *parent = 0);
	~OptionsWidget();

	int fontSize() const;
	QString chars() const;

signals:
	void fontSize_valueChanged(int);
	void chars_textChanged(const QString&);

private slots:
	void restoreDefault();

private:
	Ui::OptionsWidgetClass ui;
};

#endif // OPTIONSWIDGET_H

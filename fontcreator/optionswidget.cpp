#include "optionswidget.h"

OptionsWidget::OptionsWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	restoreDefault();

	connect(ui.fontSize, SIGNAL(valueChanged(int)), this, SIGNAL(fontSize_valueChanged(int)));
	connect(ui.chars, SIGNAL(textChanged(const QString&)), this, SIGNAL(chars_textChanged(const QString&)));
	connect(ui.restoreDefault, SIGNAL(clicked()), this, SLOT(restoreDefault()));
}

OptionsWidget::~OptionsWidget()
{

}

int OptionsWidget::fontSize() const
{
	return ui.fontSize->value();
}

QString OptionsWidget::chars() const
{
	return ui.chars->text();
}

void OptionsWidget::restoreDefault()
{
	QString chars;
	for (char c = 32; c < 127; ++c)
		chars += QChar(c);
	ui.chars->setText(chars);
}

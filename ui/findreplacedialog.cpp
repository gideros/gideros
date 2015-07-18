#include "findreplacedialog.h"

FindReplaceDialog::FindReplaceDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.findNext, SIGNAL(clicked()), this, SIGNAL(findNext()));
	connect(ui.findPrevious, SIGNAL(clicked()), this, SIGNAL(findPrevious()));
}

FindReplaceDialog::~FindReplaceDialog()
{

}

QString FindReplaceDialog::findWhat() const
{
	return ui.findWhat->text();
}

bool FindReplaceDialog::wholeWord() const
{
	return ui.wholeWord->isChecked();
}

bool FindReplaceDialog::matchCase() const
{
	return ui.matchCase->isChecked();
}

bool FindReplaceDialog::wrapAround() const
{
	return ui.wrapAround->isChecked();
}

bool FindReplaceDialog::regularExpressions() const
{
	return ui.regularExpressions->isChecked();
}

void FindReplaceDialog::setMathcesText(const QString& str)
{
	ui.matches->setText(str);
}

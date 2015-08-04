#include "finddialog.h"

FindDialog::FindDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.findNext, SIGNAL(clicked()), this, SIGNAL(findNext()));
	connect(ui.cancel, SIGNAL(clicked()), this, SLOT(close()));
}

FindDialog::~FindDialog()
{

}

QString FindDialog::findWhat() const
{
	return ui.findWhat->text();
}

bool FindDialog::wholeWord() const
{
	return ui.wholeWord->isChecked();
}

bool FindDialog::matchCase() const
{
	return ui.matchCase->isChecked();
}

bool FindDialog::forward() const
{
	return ui.down->isChecked();
}

void FindDialog::focusToFindWhat()
{
	ui.findWhat->setFocus(Qt::OtherFocusReason);
	ui.findWhat->selectAll();
}

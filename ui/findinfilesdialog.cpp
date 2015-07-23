#include "findinfilesdialog.h"

FindInFilesDialog::FindInFilesDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.findAll, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui.cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

FindInFilesDialog::~FindInFilesDialog()
{

}

QString FindInFilesDialog::findWhat() const
{
	return ui.findWhat->text();
}

QString FindInFilesDialog::filter() const
{
	return ui.filter->text();
}

bool FindInFilesDialog::wholeWord() const
{
	return ui.wholeWord->isChecked();
}

bool FindInFilesDialog::matchCase() const
{
	return ui.matchCase->isChecked();
}


void FindInFilesDialog::focusToFindWhat()
{
	ui.findWhat->setFocus(Qt::OtherFocusReason);
	ui.findWhat->selectAll();
}

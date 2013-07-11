#include "replacedialog.h"

ReplaceDialog::ReplaceDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.findNext, SIGNAL(clicked()), this, SIGNAL(findNext()));
	connect(ui.replace, SIGNAL(clicked()), this, SIGNAL(replace()));
	connect(ui.replaceAll, SIGNAL(clicked()), this, SIGNAL(replaceAll()));
	connect(ui.cancel, SIGNAL(clicked()), this, SLOT(close()));
}

ReplaceDialog::~ReplaceDialog()
{

}


QString ReplaceDialog::findWhat() const
{
	return ui.findWhat->text();
}

QString ReplaceDialog::replaceWith() const
{
	return ui.replaceWith->text();
}

bool ReplaceDialog::wholeWord() const
{
	return ui.wholeWord->isChecked();
}

bool ReplaceDialog::matchCase() const
{
	return ui.matchCase->isChecked();
}

void ReplaceDialog::focusToFindWhat()
{
	ui.findWhat->setFocus(Qt::OtherFocusReason);
	ui.findWhat->selectAll();
}

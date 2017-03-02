#include "findinfilesdialog.h"
#include <QSettings>

FindInFilesDialog::FindInFilesDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.findAll, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui.cancel, SIGNAL(clicked()), this, SLOT(reject()));
	QSettings settings;
    ui.matchCase->setChecked(settings.value("findinfilesdialog matchcase", false).toBool());
}

FindInFilesDialog::~FindInFilesDialog()
{

}

void FindInFilesDialog::hideEvent(QHideEvent * event)
{
	QDialog::hideEvent(event);
	QSettings settings;
    settings.setValue("findinfilesdialog matchcase",ui.matchCase->isChecked());
	ui.findAll->setFocus(Qt::OtherFocusReason);
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

bool FindInFilesDialog::regexp() const
{
    return ui.regexp->isChecked();
}

void FindInFilesDialog::focusToFindWhat()
{
	ui.findWhat->setFocus(Qt::OtherFocusReason);
	ui.findWhat->selectAll();
}

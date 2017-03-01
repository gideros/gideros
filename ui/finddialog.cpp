#include "finddialog.h"
#include <QSettings>

FindDialog::FindDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.findNext, SIGNAL(clicked()), this, SIGNAL(findNext()));
	connect(ui.cancel, SIGNAL(clicked()), this, SLOT(close()));
	QSettings settings;
    ui.matchCase->setChecked(settings.value("finddialog matchcase", false).toBool());
    ui.wrap->setChecked(settings.value("finddialog wrap", true).toBool());
}

FindDialog::~FindDialog()
{
}

void FindDialog::hideEvent(QHideEvent * event)
{
	QDialog::hideEvent(event);
	QSettings settings;
    settings.setValue("finddialog matchcase",ui.matchCase->isChecked());
    settings.setValue("finddialog wrap",ui.wrap->isChecked());
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

bool FindDialog::wrap() const
{
    return ui.wrap->isChecked();
}

bool FindDialog::regexp() const
{
    return ui.regexp->isChecked();
}

void FindDialog::focusToFindWhat()
{
	ui.findWhat->setFocus(Qt::OtherFocusReason);
	ui.findWhat->selectAll();
}

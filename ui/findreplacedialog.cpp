#include "findreplacedialog.h"
#include <QSettings>

FindReplaceDialog::FindReplaceDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	connect(ui.findNext, SIGNAL(clicked()), this, SIGNAL(findNext()));
	connect(ui.findPrevious, SIGNAL(clicked()), this, SIGNAL(findPrevious()));
	QSettings settings;
    ui.matchCase->setChecked(settings.value("replacedialog matchcase", false).toBool());
    ui.wrapAround->setChecked(settings.value("replacedialog wrap", true).toBool());
}

FindReplaceDialog::~FindReplaceDialog()
{

}

void FindReplaceDialog::hideEvent(QHideEvent * event)
{
	QDialog::hideEvent(event);
	QSettings settings;
    settings.setValue("replacedialog matchcase",ui.matchCase->isChecked());
    settings.setValue("replacedialog wrap",ui.wrapAround->isChecked());
	ui.findNext->setFocus(Qt::OtherFocusReason);
}

QString FindReplaceDialog::findWhat() const
{
	return ui.findWhat->text();
}

void FindReplaceDialog::setSelectedText(QString s) const
{
	ui.findWhat->setText(s);
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

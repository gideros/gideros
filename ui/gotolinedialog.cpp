#include "gotolinedialog.h"
#include <QMessageBox>

GoToLineDialog::GoToLineDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	lineNumber_ = 0;

	connect(ui.ok, SIGNAL(clicked()), this, SLOT(ok()));
	connect(ui.cancel, SIGNAL(clicked()), this, SLOT(cancel()));
}

GoToLineDialog::~GoToLineDialog()
{

}

void GoToLineDialog::setLineNumbers(int current, int max)
{
	ui.lineNumberLabel->setText(tr("Line Number (1-%1):").arg(max));

	ui.lineNumber->setText(QString::number(current));
}

void GoToLineDialog::ok()
{
	bool ok;
	lineNumber_ = ui.lineNumber->text().toInt(&ok);

	if (ok == false)
		QMessageBox::information(this, tr("Gideros"), tr("Give a proper integer value."));
	else
		accept();
}

void GoToLineDialog::cancel()
{
	reject();
}

int GoToLineDialog::lineNumber() const
{
	return lineNumber_;
}

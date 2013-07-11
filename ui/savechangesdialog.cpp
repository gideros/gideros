#include "savechangesdialog.h"

SaveChangesDialog::SaveChangesDialog(const QStringList& files, QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	ui.fileList->addItems(files);

	connect(ui.yes, SIGNAL(clicked()), this, SLOT(yes()));
	connect(ui.no, SIGNAL(clicked()), this, SLOT(no()));
	connect(ui.cancel, SIGNAL(clicked()), this, SLOT(cancel()));
}

SaveChangesDialog::~SaveChangesDialog()
{

}


void SaveChangesDialog::yes()
{
	accept();
}

void SaveChangesDialog::no()
{
	done(2);
}

void SaveChangesDialog::cancel()
{
	reject();
}

#include "addnewfiledialog.h"
#include <QFileDialog>

AddNewFileDialog::AddNewFileDialog(const QString& projectDirectory, QWidget *parent) :
	QDialog(parent),
	projectDirectory_(projectDirectory)
{
	ui.setupUi(this);
	ui.location->setText(projectDirectory);
	ui.fileName->setText("newfile.lua");
	ui.fileName->setSelection(0, 7);

	connect(ui.ok, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui.cancel, SIGNAL(clicked()), this, SLOT(reject()));
}

AddNewFileDialog::~AddNewFileDialog()
{

}

void AddNewFileDialog::on_browse_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("New File Location"), projectDirectory_);

	if (dir.isEmpty() == false)
		ui.location->setText(dir);
}

QString AddNewFileDialog::fileName() const
{
	return ui.fileName->text();
}
QString AddNewFileDialog::location() const
{
	return ui.location->text();
}

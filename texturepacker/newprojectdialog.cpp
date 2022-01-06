#include "newprojectdialog.h"
#include <QFileDialog>
#include <QSettings>

NewProjectDialog::NewProjectDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);

	QSettings settings;

	if (settings.value("location").isNull() == true)
		settings.setValue("location", QDir::homePath());

	ui.location->setText(settings.value("location").toString());

	updateWillCreateLabel();
}

NewProjectDialog::~NewProjectDialog()
{

}

QString NewProjectDialog::name() const
{
	return ui.name->text();
}

QString NewProjectDialog::location() const
{
	return ui.location->text();
}

bool NewProjectDialog::createDirectory() const
{
	return ui.createDirectory->isChecked();
}

void NewProjectDialog::on_browse_clicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Project Location"));

	if (dir.isEmpty() == false)
		ui.location->setText(dir);
}

void NewProjectDialog::on_OK_clicked()
{
	accept();
}

void NewProjectDialog::on_cancel_clicked()
{
	reject();
}

void NewProjectDialog::on_name_textChanged(const QString& text)
{
    Q_UNUSED(text);
	updateWillCreateLabel();
}

void NewProjectDialog::on_location_textChanged(const QString& text)
{
    Q_UNUSED(text);
    updateWillCreateLabel();
}

void NewProjectDialog::on_createDirectory_stateChanged(int state)
{
    Q_UNUSED(state);
    updateWillCreateLabel();
}

void NewProjectDialog::updateWillCreateLabel()
{
	ui.willCreate->setText(tr("Will create: ") + fullName());
}

QString NewProjectDialog::fullDirectory() const
{
	QDir dir(location());

	if (createDirectory())
		dir = dir.absoluteFilePath(name());

	return dir.absolutePath();
}

QString NewProjectDialog::fullName() const
{
	return QDir(fullDirectory()).absoluteFilePath(name() + ".tpproj");
}

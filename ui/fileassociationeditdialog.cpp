#include "fileassociationeditdialog.h"
#include "ui_fileassociationeditdialog.h"
#include <QFileDialog>
#include <QFileInfo>

FileAssociationEditDialog::FileAssociationEditDialog(const QString& extension,
													 const QString& applicationPath,
													 const QString& arguments,
													 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileAssociationEditDialog)
{
    ui->setupUi(this);

	ui->extension->setText(extension);
	ui->application->setText(applicationPath);
	ui->arguments->setText(arguments);
}

FileAssociationEditDialog::~FileAssociationEditDialog()
{
    delete ui;
}

void FileAssociationEditDialog::on_choose_clicked()
{
#ifdef Q_OS_MAC
	QString fileName = QFileDialog::getOpenFileName(this, tr("Choose Application"),
													"/Applications",
													tr("Applications (*.app)"));
#else
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
													QString(),
													tr("Executables (*.exe)"));
#endif

	if (fileName.isEmpty() == false)
		ui->application->setText(fileName);
}

QString FileAssociationEditDialog::extension() const
{
	return ui->extension->text();
}

QString FileAssociationEditDialog::applicationPath() const
{
	return ui->application->text();
}

QString FileAssociationEditDialog::arguments() const
{
	return ui->arguments->text();
}

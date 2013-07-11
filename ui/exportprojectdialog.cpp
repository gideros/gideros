#include "exportprojectdialog.h"
#include "ui_exportprojectdialog.h"
#include "projectproperties.h"

ExportProjectDialog::ExportProjectDialog(ProjectProperties* properties, bool licensed, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportProjectDialog)
{
	properties_ = properties;

	ui->setupUi(this);

	connect(ui->architecture, SIGNAL(currentIndexChanged(int)), ui->architectureTab, SLOT(setCurrentIndex(int)));
	connect(ui->architectureTab, SIGNAL(currentChanged(int)), ui->architecture, SLOT(setCurrentIndex(int)));

	ui->architecture->setCurrentIndex(properties_->architecture);
	ui->assetsOnly->setChecked(properties_->assetsOnly);
	ui->packageName->setText(properties_->packageName);

    if (licensed)
    {
        ui->encrypt->setEnabled(true);
        ui->encrypt->setChecked(properties_->encrypt);
    }
    else
    {
        ui->encrypt->setEnabled(false);
        ui->encrypt->setChecked(false);
    }

	connect(this, SIGNAL(accepted()), this, SLOT(onAccepted()));
}

ExportProjectDialog::~ExportProjectDialog()
{
    delete ui;
}

ExportProjectDialog::DeviceFamily ExportProjectDialog::deviceFamily() const
{
    return (DeviceFamily)ui->architecture->currentIndex();
}

QString ExportProjectDialog::packageName() const
{
	return ui->packageName->text();
}

bool ExportProjectDialog::assetsOnly() const
{
	return ui->assetsOnly->isChecked();
}

bool ExportProjectDialog::encrypt() const
{
    return ui->encrypt->isChecked();
}

void ExportProjectDialog::onAccepted()
{
	properties_->architecture = ui->architecture->currentIndex();
	properties_->assetsOnly = ui->assetsOnly->isChecked();
	properties_->packageName = ui->packageName->text();
    properties_->encrypt = ui->encrypt->isChecked();
}

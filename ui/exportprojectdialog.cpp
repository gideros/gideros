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
    ui->android_template->setCurrentIndex(properties_->android_template);
	ui->assetsOnly->setChecked(properties_->assetsOnly);
    ui->ios_bundle->setText(properties_->ios_bundle);
	ui->packageName->setText(properties_->packageName);
    ui->osx_org->setText(properties->osx_org);
    ui->osx_domain->setText(properties->osx_domain);
    ui->osx_bundle->setText(properties_->osx_bundle);
    ui->win_org->setText(properties->win_org);
    ui->win_domain->setText(properties->win_domain);
    ui->winrt_org->setText(properties->winrt_org);
    ui->winrt_package->setText(properties->winrt_package);

    if (licensed)
    {
        ui->encryptCode->setEnabled(true);
        ui->encryptCode->setChecked(properties_->encryptCode);
        ui->encryptAssets->setEnabled(true);
        ui->encryptAssets->setChecked(properties_->encryptAssets);
    }
    else
    {
        ui->encryptCode->setEnabled(false);
        ui->encryptCode->setChecked(false);
        ui->encryptAssets->setEnabled(false);
        ui->encryptAssets->setChecked(false);
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

QString ExportProjectDialog::ios_bundle() const
{
    return ui->ios_bundle->text();
}

QString ExportProjectDialog::packageName() const
{
    return ui->packageName->text();
}

QString ExportProjectDialog::androidTemplate() const
{
    return ui->android_template->currentText();
}

QString ExportProjectDialog::osx_org() const
{
    return ui->osx_org->text();
}

QString ExportProjectDialog::osx_domain() const
{
    return ui->osx_domain->text();
}

QString ExportProjectDialog::osx_bundle() const
{
    return ui->osx_bundle->text();
}

QString ExportProjectDialog::win_org() const
{
    return ui->win_org->text();
}

QString ExportProjectDialog::win_domain() const
{
    return ui->win_domain->text();
}

QString ExportProjectDialog::winrt_org() const
{
    return ui->winrt_org->text();
}

QString ExportProjectDialog::winrt_package() const
{
    return ui->winrt_package->text();
}

bool ExportProjectDialog::assetsOnly() const
{
	return ui->assetsOnly->isChecked();
}

bool ExportProjectDialog::encryptCode() const
{
    return ui->encryptCode->isChecked();
}

bool ExportProjectDialog::encryptAssets() const
{
    return ui->encryptAssets->isChecked();
}

void ExportProjectDialog::onAccepted()
{
	properties_->architecture = ui->architecture->currentIndex();
    properties_->android_template = ui->android_template->currentIndex();
	properties_->assetsOnly = ui->assetsOnly->isChecked();
    properties_->ios_bundle = ui->ios_bundle->text();
	properties_->packageName = ui->packageName->text();
    properties_->osx_org = ui->osx_org->text();
    properties_->osx_domain = ui->osx_domain->text();
    properties_->osx_bundle = ui->osx_bundle->text();
    properties_->win_org = ui->win_org->text();
    properties_->win_domain = ui->win_domain->text();
    properties_->winrt_org = ui->winrt_org->text();
    properties_->winrt_package = ui->winrt_package->text();
    properties_->encryptCode = ui->encryptCode->isChecked();
    properties_->encryptAssets = ui->encryptAssets->isChecked();
}

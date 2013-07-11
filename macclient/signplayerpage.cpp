#include "signplayerpage.h"
#include "ui_signplayerpage.h"
#include <QFileDialog>

SignPlayerPage::SignPlayerPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::SignPlayerPage)
{
    ui->setupUi(this);

//	setTitle("iPhone Player Signer");
//	setSubTitle("Signing and downloading iPhone player");

	registerField("privateKey*", ui->privateKey);
	registerField("provisioningProfile*", ui->provisioningProfile);
	registerField("developerSigningCertificate*", ui->developerSigningCertificate);
	registerField("output*", ui->output);
}

SignPlayerPage::~SignPlayerPage()
{
    delete ui;
}

void SignPlayerPage::on_browsePrivateKey_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
													QString(),
													tr("Private Key Files (*.key *.p12)"));
	if (!fileName.isEmpty())
		ui->privateKey->setText(fileName);
}

void SignPlayerPage::on_browseProvisioningProfile_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
													QString(),
													tr("Mobile Provision Files (*.mobileprovision)"));
	if (!fileName.isEmpty())
		ui->provisioningProfile->setText(fileName);
}

void SignPlayerPage::on_browseDeveloperSigningCertificate_clicked()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
													QString(),
													tr("Signing Certificate Files (*.cer)"));
	if (!fileName.isEmpty())
		ui->developerSigningCertificate->setText(fileName);
}


void SignPlayerPage::on_browseOutput_clicked()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
													QString("iPhonePlayer.ipa"),
													tr("iPhone Application Archive Files (*.ipa)"));
	if (!fileName.isEmpty())
		ui->output->setText(fileName);
}

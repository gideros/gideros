#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#define GIDEROS_VERSION_ONLY
#include "gideros.h"
#undef GIDEROS_VERSION_ONLY

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

	QString giderosVersion=QString(GIDEROS_VERSION);
    ui->labelVersion->setText(giderosVersion);
#ifdef GIDEROS_GIT_HASH
	QString giderosHash=QString(GIDEROS_GIT_HASH);
    ui->labelHash->setText(giderosHash);
#endif
    QString cptext=ui->labelCopyright->text();
    cptext.replace(QString("ENDYEAR"), giderosVersion.split(".")[0]);
    ui->labelCopyright->setText(cptext);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

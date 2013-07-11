#include "dialog.h"
#include "ui_dialog.h"
#include "startpage.h"
#include "signplayerpage.h"
#include "signplayerrunpage.h"

Dialog::Dialog(QWidget *parent) :
	QWizard(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
//	addPage(new StartPage);
	addPage(new SignPlayerPage);
	addPage(new SignPlayerRunPage);
}

Dialog::~Dialog()
{
    delete ui;
}

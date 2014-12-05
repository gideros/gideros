#include "projectpropertiesdialog.h"
#include "ui_projectpropertiesdialog.h"

ProjectPropertiesDialog::ProjectPropertiesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProjectPropertiesDialog)
{
    ui->setupUi(this);
}

ProjectPropertiesDialog::~ProjectPropertiesDialog()
{
    delete ui;
}

#ifndef PROJECTPROPERTIESDIALOG_H
#define PROJECTPROPERTIESDIALOG_H

#include <QDialog>

namespace Ui {
    class ProjectPropertiesDialog;
}

class ProjectPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectPropertiesDialog(QWidget *parent = 0);
    ~ProjectPropertiesDialog();

private:
    Ui::ProjectPropertiesDialog *ui;
};

#endif // PROJECTPROPERTIESDIALOG_H

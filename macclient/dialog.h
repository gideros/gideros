#ifndef DIALOG_H
#define DIALOG_H

#include <QWizard>

namespace Ui {
    class Dialog;
}

class Dialog : public QWizard
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H

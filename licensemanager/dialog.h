#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

namespace Ui {
    class Dialog;
}

class ActivateWidget;
class DeactivateWidget;


class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();


private slots:
	void updateUI();

private:
    Ui::Dialog *ui;

private:
	ActivateWidget* activateWidget_;
	DeactivateWidget* deactivateWidget_;
};

#endif // DIALOG_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class ActivateWidget;
class DeactivateWidget;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
	void updateUI();
	void deactivateAll();

private:
    Ui::MainWindow *ui;

private:
	void checkExpired();

private:
	ActivateWidget* activateWidget_;
	DeactivateWidget* deactivateWidget_;
};

#endif // MAINWINDOW_H

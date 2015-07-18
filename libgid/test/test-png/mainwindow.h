#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
protected:
    virtual void paintEvent(QPaintEvent *);

private:
    Ui::MainWindow *ui;
    QImage image;
    QImage background;
};

#endif // MAINWINDOW_H

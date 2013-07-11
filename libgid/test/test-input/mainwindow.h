#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGLWidget>

namespace Ui {
class MainWindow;
}

class GLWidget : public QGLWidget
{
    Q_OBJECT

protected:
    ~GLWidget();
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);

private:
};


class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    virtual void timerEvent(QTimerEvent *);

    static void callback_s(int type, void *event, void *data);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

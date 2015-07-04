#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

class GLCanvas;
class QActionGroup;

enum Scale{
    eZoomIn,
    eZoomOut,
    eFitToWindow
};

enum Rotate{
    eLeft,
    eRight
};

class MainWindow : public QMainWindow{
	Q_OBJECT

    public:
        MainWindow(QWidget *parent = 0);
        ~MainWindow();

        static MainWindow* getInstance(){
            return instance;
        }

        void fullScreenWindow(bool _fullScreen);
        void resizeWindow(int width, int height);
        void updateResolution();
        float deviceScale();
        float scale();
        void setScale(float scale);

    private:
        static MainWindow* instance;
        GLCanvas *glCanvas;
        float scale_;

        Ui::MainWindowClass ui;

    private slots:
        void projectNameChanged(const QString& projectName);
};

#endif // MAINWINDOW_H

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
        void setResolution(int width, int height);

    private:
        static MainWindow* instance;
        GLCanvas *glCanvas;
        bool fullscreen_;
        int width_;
        int height_;

        Ui::MainWindowClass ui;

    private slots:
        void projectNameChanged(const QString& projectName);

    protected:
        virtual void resizeEvent(QResizeEvent*);
};

#endif // MAINWINDOW_H

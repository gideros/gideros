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
        void updateResolution(bool event);
        float deviceScale();
        float scale();
        void setScale(float scale);
        void setFixedSize(bool fixedSize);
        void setLogicalScaleMode(LogicalScaleMode scaleMode);
        QSize windowSize();
        void printToOutput(const char* text);
        bool fullScreen();

    private:
        static MainWindow* instance;
        GLCanvas *glCanvas;
        float scale_;
        bool fixedSize_;
        int width_;
        int height_;
        float resolution_;
        int width0_;
        int height0_;
        int scaleModeNum_;
        bool fullScreen_;

        Ui::MainWindowClass ui;

    private slots:
        void projectNameChanged(const QString& projectName);

    protected:
        virtual void closeEvent(QCloseEvent*);
        virtual void resizeEvent(QResizeEvent*);
        virtual void changeEvent(QEvent*);
};

#endif // MAINWINDOW_H

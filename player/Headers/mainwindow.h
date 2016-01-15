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

        int width();
        int height();
        int fps();
        bool autoScale();
        bool alwaysOnTop();
        Orientation orientation();
        float scale();
        float deviceScale();
        bool drawInfos();
        bool fullScreen();
        bool hideMenu();
        QColor backgroundColor();
        QColor canvasColor();
        QColor infoColor();

        void setWidth(int width);
        void setHeight(int height);
        void setFps(int fps);
        void setAutoScale(bool autoScale);
        void setAlwaysOnTop(bool alwaysOnTop);
        void setOrientation(Orientation orientation);
        void setScale(float scale);
        void setDrawInfos(bool drawInfos);
        void setHideMenu(bool hideMenu);
        void setFullScreen(bool fullScreen);
        void setBackgroundColor(QColor backgroundColor);
        void setCanvasColor(QColor canvasColor);
        void setInfoColor(QColor infoColor);

        void updateResolution(bool event);
        void updateAutoScale();
        void updateAlwaysOnTop();
        void updateFps();
        void updateOrientation();
        void updateDrawInfos();
        void updateFullScreen();
        void updateHideMenu();
        void updateBackgroundColor();
        void updateCanvasColor();
        void updateInfoColor();
        void checkLoadedSettings();
        void saveSettings();
        void resizeWindow(int width, int height);
        void fullScreenWindow(bool _fullScreen);
        void setFixedSize(bool fixedSize);
        QSize windowSize();
        void printToOutput(const char* text);

        static MainWindow* getInstance(){
            return instance;
        }

    protected:
        virtual void closeEvent(QCloseEvent*);
        virtual void resizeEvent(QResizeEvent*);

    private:
        static MainWindow* instance;
        void setupUiActions();
        void setupUiProperties();
        void createUiGroups();
        void loadSettings();
        QString getWorkingDirectory();

        Ui::MainWindowClass ui;
        QActionGroup* resolutionGroup_;
        QActionGroup* fpsGroup_;
        QActionGroup* orientationGroup_;
        int width_;
        int height_;
        int fps_;
        bool autoScale_;
        bool alwaysOnTop_;
        float scale_;
        bool hideMenu_;
        bool fullScreen_;
        Orientation orientation_;
        bool drawInfos_;
        QColor backgroundColor_;
        QColor canvasColor_;
        QColor infoColor_;
        QString projectName_;

    private slots:
        void actionFull_Screen(bool checked);
        void actionHide_Menu();
        void actionAlwaysOnTop(bool checked);
        void actionAuto_Scale(bool checked);
        void actionResolution();
        void actionFps();
        void actionOrientation();
        void actionScale();
        void actionFitWindow();
        void actionRotate();
        void actionSettings();
        void actionDraw_Infos(bool checked);
        void actionOpen();
        void actionOpen_Directory();
        void actionRestart();
        void projectNameChanged(const QString& projectName);
};

#endif // MAINWINDOW_H

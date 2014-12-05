#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

class GLCanvas;
class QActionGroup;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
	~MainWindow();

protected:
    // protected actions based on events like close, resize, ...
    virtual void closeEvent(QCloseEvent*);
    virtual void resizeEvent(QResizeEvent*);

private slots:
    void setupUi();
    void setupGroups();
    void loadSettings();
    void saveSettings();
    void loadResolution(int resolution);
    void loadScale(int scale);
    void loadOrientation(Orientation orientation);
    void loadFps(int fps);

    void actionOpen();

    void actionFull_Screen(bool checked);
    void actionHide_Menu(bool checked);
    void alwaysOnTop(bool checked);
    void actionScale();

    void actionAuto_Scale(bool checked);
	void rotateLeft();
	void rotateRight();
	void portrait();
	void portraitUpsideDown();
	void landscapeLeft();
	void landscapeRight();
    void actionResolution();
	void action15_fps();
	void action30_fps();
	void action60_fps();
	void actionUnlimited();

    void sendRun();

	void projectNameChanged(const QString& projectName);

private:
    Ui::MainWindowClass ui;
    QActionGroup* orientationGroup_;
    QActionGroup* resolutionGroup_;
    QActionGroup* zoomGroup_;

    int scale();
	int hardwareWidth();
	int hardwareHeight();
    Orientation orientation() const;
};

#endif // MAINWINDOW_H

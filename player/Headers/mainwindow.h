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
	virtual void closeEvent(QCloseEvent* event);

private slots:
	void rotateLeft();
	void rotateRight();
	void portrait();
	void portraitUpsideDown();
	void landscapeLeft();
	void landscapeRight();
	void alwaysOnTop(bool checked);

	void action15_fps();
	void action30_fps();
	void action60_fps();
	void actionUnlimited();

	void actionResolution();

    //void exportAccessedFiles();

	void sendRun();

	void actionScale();

private slots:
	void afterInitialization();

private slots:
	void projectNameChanged(const QString& projectName);

private:
	Orientation orientation() const;
	int hardwareWidth();
	int hardwareHeight();
	int scale();

private:
	Ui::MainWindowClass ui;

private:
	QActionGroup* orientationGroup_;
	QActionGroup* resolutionGroup_;
	QActionGroup* zoomGroup_;
};

#endif // MAINWINDOW_H

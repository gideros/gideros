#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
class QDockWidget;
class LibraryTreeWidget;
class OptionsWidget;
class Canvas;
class QLabel;
class QSplitter;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
	~MainWindow();
    void openStartProject(const QString& fileName);


private slots:
	void onUpdateTexture();
	void onExportTexture();

private slots:
	void newProject();
	void openProject();
	void saveProject();
	void closeProject();
	void openProject(const QString& fileName);

private:
	bool maybeSave();
	void closeProject(int);

private:
	Ui::MainWindowClass ui;

private:
	QDockWidget* libraryDock_;
//	QDockWidget* optionsDock_;
	LibraryTreeWidget* libraryWidget_;
	OptionsWidget* optionsWidget_;
	Canvas* canvas_;
	QLabel* libraryLabel_;
	QSplitter* splitter1_;

protected:
	void closeEvent(QCloseEvent* event);

private:
	QString projectFileName_;

private:
	void updateUI();

private:
	void addToRecentProjects(const QString& fileName);
	void updateRecentProjectActions();

private slots:
	void openRecentProject();
	void setLibraryWidgetModified(bool);
};

#endif // MAINWINDOW_H

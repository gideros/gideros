#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"

class OptionsWidget;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	Ui::MainWindowClass ui;

private:
	QString fontFile_;
	OptionsWidget* optionsWidget_;

private slots:
    void importFont();
    void exportFont();
};

#endif // MAINWINDOW_H

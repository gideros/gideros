#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "requester.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

	requests_.push_back(req_load("http://localhost", s_callback, this));

	startTimer(10);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::s_callback(int id, int res, void* ptr, size_t size, void* data)
{
	static_cast<MainWindow*>(data)->callback(id, res, ptr, size);
}

void MainWindow::callback(int id, int res, void* ptr, size_t size)
{
	qDebug() << "callback" << id << res;
	req_delete(id);
}

void MainWindow::timerEvent(QTimerEvent *)
{
	req_tick();
}

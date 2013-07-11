#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <ghttp.h>
#include <QDebug>

void callback_s(int type, void *event, void *udata)
{
    qDebug() << type;
    if (type == GHTTP_RESPONSE_EVENT)
    {
        ghttp_ResponseEvent* event2 = (ghttp_ResponseEvent*)event;
        qDebug() << (char*)event2->data;
    }

}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    startTimer(16);

    ghttp_Get("http://jenots.com/", NULL, callback_s, NULL);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *)
{
    gevent_Tick();
}


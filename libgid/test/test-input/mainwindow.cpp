#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <ginput.h>
#include <ginput-qt.h>
#include <gevent.h>
#include <glog.h>
#include <QMouseEvent>

GLWidget::~GLWidget()
{
}

void GLWidget::initializeGL()
{
    glClearColor(0.2, 0.4, 0.6, 1.0);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
}

void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLint)w, (GLint)h);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
}

void GLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
}

void GLWidget::mousePressEvent(QMouseEvent *e)
{
    int button;
    switch (e->button())
    {
    case Qt::LeftButton:
        button = GINPUT_LEFT_BUTTON;
        break;
    case Qt::RightButton:
        button = GINPUT_RIGHT_BUTTON;
        break;
    case Qt::MiddleButton:
        button = GINPUT_MIDDLE_BUTTON;
        break;
    default:
        button = GINPUT_NO_BUTTON;
    }

    ginputp_mouseDown(e->x(), e->y(), button);
}

void GLWidget::mouseMoveEvent(QMouseEvent *e)
{
    ginputp_mouseMove(e->x(), e->y());
}

void GLWidget::mouseReleaseEvent(QMouseEvent *e)
{
    int button;
    switch (e->button())
    {
    case Qt::LeftButton:
        button = GINPUT_LEFT_BUTTON;
        break;
    case Qt::RightButton:
        button = GINPUT_RIGHT_BUTTON;
        break;
    case Qt::MiddleButton:
        button = GINPUT_MIDDLE_BUTTON;
        break;
    default:
        button = GINPUT_NO_BUTTON;
    }

    ginputp_mouseUp(e->x(), e->y(), button);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(new GLWidget);

    gevent_addGlobalCallback(GEVENT_PRE_TICK_EVENT, callback_s, this);
    gevent_addGlobalCallback(GEVENT_POST_TICK_EVENT, callback_s, this);
    gevent_addGlobalCallback(GINPUT_MOUSE_DOWN_EVENT, callback_s, this);
    gevent_addGlobalCallback(GINPUT_MOUSE_MOVE_EVENT, callback_s, this);
    gevent_addGlobalCallback(GINPUT_MOUSE_UP_EVENT, callback_s, this);

    startTimer(16);
}

void MainWindow::callback_s(int type, void *event, void *data)
{
    if (type == GEVENT_PRE_TICK_EVENT)
    {
    }
    else if (type == GEVENT_POST_TICK_EVENT)
    {
    }
    else if (type == GINPUT_MOUSE_DOWN_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        glog_d("down %d %d %d", event2->x, event2->y, event2->button);
    }
    else if (type == GINPUT_MOUSE_MOVE_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        glog_d("move %d %d %d", event2->x, event2->y, event2->button);
    }
    else if (type == GINPUT_MOUSE_UP_EVENT)
    {
        ginput_MouseEvent *event2 = (ginput_MouseEvent*)event;
        glog_d("up %d %d %d", event2->x, event2->y, event2->button);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *)
{
    gevent_Tick();
}

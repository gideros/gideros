#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <gimage.h>
#include <gtexture.h>
#include <QDebug>

GLWidget::~GLWidget()
{
    gtexture_cleanup();
}

void GLWidget::initializeGL()
{
    gtexture_init();
    gtexture_setCachingEnabled(1);

    glClearColor(0.2, 0.4, 0.6, 1.0);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    int width, height, comp;
    int result = gimage_parsePng("sky_world.png", &width, &height, &comp);
    qDebug() << result << width << height << comp;

    void *pixels = malloc(width * height * comp);
    gimage_loadPng("sky_world.png", pixels);

    texture_ = gtexture_create(width, height,
                               GTEXTURE_RGB, GTEXTURE_UNSIGNED_BYTE,
                               GTEXTURE_CLAMP, GTEXTURE_NEAREST,
                               pixels,
                               "sky_world.png", strlen("sky_world.png") + 1);
    gtexture_tick();

    GLuint id = gtexture_getInternalId(texture_);
    glDeleteTextures(1, &id);

    gtexture_reloadTextures();
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
    gtexture_tick();

    glClear(GL_COLOR_BUFFER_BIT);

    GLuint id = gtexture_getInternalId(texture_);

    glBindTexture(GL_TEXTURE_2D, id);

    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    glTexCoord2f(1, 0);
    glVertex2f(1, 0);
    glTexCoord2f(1, 1);
    glVertex2f(1, 1);
    glTexCoord2f(0, 1);
    glVertex2f(0, 1);
    glEnd();
}



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setCentralWidget(new GLWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}




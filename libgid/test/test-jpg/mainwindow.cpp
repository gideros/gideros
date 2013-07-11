#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <gimage.h>
#include <stdio.h>
#include <gstdio.h>

#include <QDebug>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    const char *pathname = "mozilla-test-jpeg/jpg-cmyk-2.jpg";

    int width, height, comp;
    int result = gimage_parseJpg(pathname, &width, &height, &comp);
    if (result == GIMAGE_NO_ERROR)
    {
        qDebug() << width << height << comp;
        std::vector<unsigned char> buf(width * height * comp);
        gimage_loadJpg(pathname, &buf[0]);

        switch (comp)
        {
        case 1:
            qDebug() << "grayscale";
            break;
        case 2:
            qDebug() << "grayscale + alpha";
            break;
        case 3:
            qDebug() << "rgb";
            break;
        case 4:
            qDebug() << "rgb + alpha";
            break;
        default:
            qDebug() << "whoa?";
            break;
        }

        image = QImage(width, height, QImage::Format_ARGB32);

        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
            {
                if (comp == 1)
                {
                    int index = x + y * width;
                    unsigned char c = buf[index];
                    image.setPixel(x, y, qRgba(c, c, c, 255));
                }
                else if (comp == 2)
                {
                    int index = (x + y * width) * 2;
                    unsigned char c = buf[index];
                    unsigned char a = buf[index + 1];
                    image.setPixel(x, y, qRgba(c, c, c, a));
                }
                else if (comp == 3)
                {
                    int index = (x + y * width) * 3;
                    unsigned char r = buf[index];
                    unsigned char g = buf[index + 1];
                    unsigned char b = buf[index + 2];
                    image.setPixel(x, y, qRgba(r, g, b, 255));
                }
                else if (comp == 4)
                {
                    int index = (x + y * width) * 4;
                    unsigned char r = buf[index];
                    unsigned char g = buf[index + 1];
                    unsigned char b = buf[index + 2];
                    unsigned char a = buf[index + 3];
                    image.setPixel(x, y, qRgba(r, g, b, a));
                }
            }

        background = QImage(width, height, QImage::Format_ARGB32);
        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
            {
                int c = ((x / 2) & 2) ^ ((y / 2) & 2);
                int c2 = c ? 192 : 224;
                background.setPixel(x, y, qRgba(c2, c2, c2, 255));
            }
    }
    else
        qDebug() << result;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QRect rect(0, 0, image.width() * 8, image.height() * 8);
    painter.drawImage(rect, background);
    painter.drawImage(rect, image);
}

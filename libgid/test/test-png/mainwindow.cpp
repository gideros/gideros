#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <gimage.h>
#include <stdio.h>
#include <gstdio.h>

#include <io.h>
#include <fcntl.h>
#include <errno.h>

#include <QDebug>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Basic formats
    const char *pathname = "PngSuite/basn0g01.png"; // - black & white
    //const char *pathname = "PngSuite/basn0g02.png"; // - 2 bit (4 level) grayscale
    //const char *pathname = "PngSuite/basn0g04.png"; // - 4 bit (16 level) grayscale
    //const char *pathname = "PngSuite/basn0g08.png"; // - 8 bit (256 level) grayscale
    //const char *pathname = "PngSuite/basn0g16.png"; // - 16 bit (64k level) grayscale
    //const char *pathname = "PngSuite/basn2c08.png"; // - 3x8 bits rgb color
    //const char *pathname = "PngSuite/basn2c16.png"; // - 3x16 bits rgb color
    //const char *pathname = "PngSuite/basn3p01.png"; // - 1 bit (2 color) paletted
    //const char *pathname = "PngSuite/basn3p02.png"; // - 2 bit (4 color) paletted
    //const char *pathname = "PngSuite/basn3p04.png"; // - 4 bit (16 color) paletted
    //const char *pathname = "PngSuite/basn3p08.png"; // - 8 bit (256 color) paletted
    //const char *pathname = "PngSuite/basn4a08.png"; // - 8 bit grayscale + 8 bit alpha-channel
    //const char *pathname = "PngSuite/basn4a16.png"; // - 16 bit grayscale + 16 bit alpha-channel
    //const char *pathname = "PngSuite/basn6a08.png"; // - 3x8 bits rgb color + 8 bit alpha-channel
    //const char *pathname = "PngSuite/basn6a16.png"; // - 3x16 bits rgb color + 16 bit alpha-channel

    // Interlacing
    //const char *pathname = "PngSuite/basi0g01.png"; // - black & white
    //const char *pathname = "PngSuite/basi0g02.png"; // - 2 bit (4 level) grayscale
    //const char *pathname = "PngSuite/basi0g04.png"; // - 4 bit (16 level) grayscale
    //const char *pathname = "PngSuite/basi0g08.png"; // - 8 bit (256 level) grayscale
    //const char *pathname = "PngSuite/basi0g16.png"; // - 16 bit (64k level) grayscale
    //const char *pathname = "PngSuite/basi2c08.png"; // - 3x8 bits rgb color
    //const char *pathname = "PngSuite/basi2c16.png"; // - 3x16 bits rgb color
    //const char *pathname = "PngSuite/basi3p01.png"; // - 1 bit (2 color) paletted
    //const char *pathname = "PngSuite/basi3p02.png"; // - 2 bit (4 color) paletted
    //const char *pathname = "PngSuite/basi3p04.png"; // - 4 bit (16 color) paletted
    //const char *pathname = "PngSuite/basi3p08.png"; // - 8 bit (256 color) paletted
    //const char *pathname = "PngSuite/basi4a08.png"; // - 8 bit grayscale + 8 bit alpha-channel
    //const char *pathname = "PngSuite/basi4a16.png"; // - 16 bit grayscale + 16 bit alpha-channel
    //const char *pathname = "PngSuite/basi6a08.png"; // - 3x8 bits rgb color + 8 bit alpha-channel
    //const char *pathname = "PngSuite/basi6a16.png"; // - 3x16 bits rgb color + 16 bit alpha-channel

    // Odd sizes
    //const char *pathname = "PngSuite/s01i3p01.png"; // - 1x1 paletted file, interlaced
    //const char *pathname = "PngSuite/s01n3p01.png"; // - 1x1 paletted file, no interlacing
    //const char *pathname = "PngSuite/s02i3p01.png"; // - 2x2 paletted file, interlaced
    //const char *pathname = "PngSuite/s02n3p01.png"; // - 2x2 paletted file, no interlacing
    //const char *pathname = "PngSuite/s03i3p01.png"; // - 3x3 paletted file, interlaced
    //const char *pathname = "PngSuite/s03n3p01.png"; // - 3x3 paletted file, no interlacing
    //const char *pathname = "PngSuite/s04i3p01.png"; // - 4x4 paletted file, interlaced
    //const char *pathname = "PngSuite/s04n3p01.png"; // - 4x4 paletted file, no interlacing
    //const char *pathname = "PngSuite/s05i3p02.png"; // - 5x5 paletted file, interlaced
    //const char *pathname = "PngSuite/s05n3p02.png"; // - 5x5 paletted file, no interlacing
    //const char *pathname = "PngSuite/s06i3p02.png"; // - 6x6 paletted file, interlaced
    //const char *pathname = "PngSuite/s06n3p02.png"; // - 6x6 paletted file, no interlacing
    //const char *pathname = "PngSuite/s07i3p02.png"; // - 7x7 paletted file, interlaced
    //const char *pathname = "PngSuite/s07n3p02.png"; // - 7x7 paletted file, no interlacing
    //const char *pathname = "PngSuite/s08i3p02.png"; // - 8x8 paletted file, interlaced
    //const char *pathname = "PngSuite/s08n3p02.png"; // - 8x8 paletted file, no interlacing
    //const char *pathname = "PngSuite/s09i3p02.png"; // - 9x9 paletted file, interlaced
    //const char *pathname = "PngSuite/s09n3p02.png"; // - 9x9 paletted file, no interlacing
    //const char *pathname = "PngSuite/s32i3p04.png"; // - 32x32 paletted file, interlaced
    //const char *pathname = "PngSuite/s32n3p04.png"; // - 32x32 paletted file, no interlacing
    //const char *pathname = "PngSuite/s33i3p04.png"; // - 33x33 paletted file, interlaced
    //const char *pathname = "PngSuite/s33n3p04.png"; // - 33x33 paletted file, no interlacing
    //const char *pathname = "PngSuite/s34i3p04.png"; // - 34x34 paletted file, interlaced
    //const char *pathname = "PngSuite/s34n3p04.png"; // - 34x34 paletted file, no interlacing
    //const char *pathname = "PngSuite/s35i3p04.png"; // - 35x35 paletted file, interlaced
    //const char *pathname = "PngSuite/s35n3p04.png"; // - 35x35 paletted file, no interlacing
    //const char *pathname = "PngSuite/s36i3p04.png"; // - 36x36 paletted file, interlaced
    //const char *pathname = "PngSuite/s36n3p04.png"; // - 36x36 paletted file, no interlacing
    //const char *pathname = "PngSuite/s37i3p04.png"; // - 37x37 paletted file, interlaced
    //const char *pathname = "PngSuite/s37n3p04.png"; // - 37x37 paletted file, no interlacing
    //const char *pathname = "PngSuite/s38i3p04.png"; // - 38x38 paletted file, interlaced
    //const char *pathname = "PngSuite/s38n3p04.png"; // - 38x38 paletted file, no interlacing
    //const char *pathname = "PngSuite/s39i3p04.png"; // - 39x39 paletted file, interlaced
    //const char *pathname = "PngSuite/s39n3p04.png"; // - 39x39 paletted file, no interlacing
    //const char *pathname = "PngSuite/s40i3p04.png"; // - 40x40 paletted file, interlaced
    //const char *pathname = "PngSuite/s40n3p04.png"; // - 40x40 paletted file, no interlacing

    // Background colors
    //const char *pathname = "PngSuite/bgai4a08.png"; // - 8 bit grayscale, alpha, no background chunk, interlaced
    //const char *pathname = "PngSuite/bgai4a16.png"; // - 16 bit grayscale, alpha, no background chunk, interlaced
    //const char *pathname = "PngSuite/bgan6a08.png"; // - 3x8 bits rgb color, alpha, no background chunk
    //const char *pathname = "PngSuite/bgan6a16.png"; // - 3x16 bits rgb color, alpha, no background chunk
    //const char *pathname = "PngSuite/bgbn4a08.png"; // - 8 bit grayscale, alpha, black background chunk
    //const char *pathname = "PngSuite/bggn4a16.png"; // - 16 bit grayscale, alpha, gray background chunk
    //const char *pathname = "PngSuite/bgwn6a08.png"; // - 3x8 bits rgb color, alpha, white background chunk
    //const char *pathname = "PngSuite/bgyn6a16.png"; // - 3x16 bits rgb color, alpha, yellow background chunk

    // Transparency
    //const char *pathname = "PngSuite/tbbn0g04.png"; // - transparent, black background chunk
    //const char *pathname = "PngSuite/tbbn2c16.png"; // - transparent, blue background chunk
    //const char *pathname = "PngSuite/tbbn3p08.png"; // - transparent, black background chunk
    //const char *pathname = "PngSuite/tbgn2c16.png"; // - transparent, green background chunk
    //const char *pathname = "PngSuite/tbgn3p08.png"; // - transparent, light-gray background chunk
    //const char *pathname = "PngSuite/tbrn2c08.png"; // - transparent, red background chunk
    //const char *pathname = "PngSuite/tbwn0g16.png"; // - transparent, white background chunk
    //const char *pathname = "PngSuite/tbwn3p08.png"; // - transparent, white background chunk
    //const char *pathname = "PngSuite/tbyn3p08.png"; // - transparent, yellow background chunk
    //const char *pathname = "PngSuite/tp0n0g08.png"; // - not transparent for reference (logo on gray)
    //const char *pathname = "PngSuite/tp0n2c08.png"; // - not transparent for reference (logo on gray)
    //const char *pathname = "PngSuite/tp0n3p08.png"; // - not transparent for reference (logo on gray)
    //const char *pathname = "PngSuite/tp1n3p08.png"; // - transparent, but no background chunk

    // Gamma values (we ignore gamma)
    //const char *pathname = "PngSuite/g03n0g16.png"; // - grayscale, file-gamma = 0.35
    //const char *pathname = "PngSuite/g03n2c08.png"; // - color, file-gamma = 0.35
    //const char *pathname = "PngSuite/g03n3p04.png"; // - paletted, file-gamma = 0.35
    //const char *pathname = "PngSuite/g04n0g16.png"; // - grayscale, file-gamma = 0.45
    //const char *pathname = "PngSuite/g04n2c08.png"; // - color, file-gamma = 0.45
    //const char *pathname = "PngSuite/g04n3p04.png"; // - paletted, file-gamma = 0.45
    //const char *pathname = "PngSuite/g05n0g16.png"; // - grayscale, file-gamma = 0.55
    //const char *pathname = "PngSuite/g05n2c08.png"; // - color, file-gamma = 0.55
    //const char *pathname = "PngSuite/g05n3p04.png"; // - paletted, file-gamma = 0.55
    //const char *pathname = "PngSuite/g07n0g16.png"; // - grayscale, file-gamma = 0.70
    //const char *pathname = "PngSuite/g07n2c08.png"; // - color, file-gamma = 0.70
    //const char *pathname = "PngSuite/g07n3p04.png"; // - paletted, file-gamma = 0.70
    //const char *pathname = "PngSuite/g10n0g16.png"; // - grayscale, file-gamma = 1.00
    //const char *pathname = "PngSuite/g10n2c08.png"; // - color, file-gamma = 1.00
    //const char *pathname = "PngSuite/g10n3p04.png"; // - paletted, file-gamma = 1.00
    //const char *pathname = "PngSuite/g25n0g16.png"; // - grayscale, file-gamma = 2.50
    //const char *pathname = "PngSuite/g25n2c08.png"; // - color, file-gamma = 2.50
    //const char *pathname = "PngSuite/g25n3p04.png"; // - paletted, file-gamma = 2.50

    // Image filtering
    //const char *pathname = "PngSuite/f00n0g08.png"; // - grayscale, no interlacing, filter-type 0
    //const char *pathname = "PngSuite/f00n2c08.png"; // - color, no interlacing, filter-type 0
    //const char *pathname = "PngSuite/f01n0g08.png"; // - grayscale, no interlacing, filter-type 1
    //const char *pathname = "PngSuite/f01n2c08.png"; // - color, no interlacing, filter-type 1
    //const char *pathname = "PngSuite/f02n0g08.png"; // - grayscale, no interlacing, filter-type 2
    //const char *pathname = "PngSuite/f02n2c08.png"; // - color, no interlacing, filter-type 2
    //const char *pathname = "PngSuite/f03n0g08.png"; // - grayscale, no interlacing, filter-type 3
    //const char *pathname = "PngSuite/f03n2c08.png"; // - color, no interlacing, filter-type 3
    //const char *pathname = "PngSuite/f04n0g08.png"; // - grayscale, no interlacing, filter-type 4
    //const char *pathname = "PngSuite/f04n2c08.png"; // - color, no interlacing, filter-type 4
    //const char *pathname = "PngSuite/f99n0g04.png"; // - bit-depth 4, filter changing per scanline

    // Additional palettes
    //const char *pathname = "PngSuite/pp0n2c16.png"; // - six-cube palette-chunk in true-color image
    //const char *pathname = "PngSuite/pp0n6a08.png"; // - six-cube palette-chunk in true-color+alpha image
    //const char *pathname = "PngSuite/ps1n0g08.png"; // - six-cube suggested palette (1 byte) in grayscale image
    //const char *pathname = "PngSuite/ps1n2c16.png"; // - six-cube suggested palette (1 byte) in true-color image
    //const char *pathname = "PngSuite/ps2n0g08.png"; // - six-cube suggested palette (2 bytes) in grayscale image
    //const char *pathname = "PngSuite/ps2n2c16.png"; // - six-cube suggested palette (2 bytes) in true-color image

    // Ancillary chunks
    //const char *pathname = "PngSuite/ccwn2c08.png"; // - chroma chunk w:0.3127,0.3290 r:0.64,0.33 g:0.30,0.60 b:0.15,0.06
    //const char *pathname = "PngSuite/ccwn3p08.png"; // - chroma chunk w:0.3127,0.3290 r:0.64,0.33 g:0.30,0.60 b:0.15,0.06
    //const char *pathname = "PngSuite/cdfn2c08.png"; // - physical pixel dimensions, 8x32 flat pixels
    //const char *pathname = "PngSuite/cdhn2c08.png"; // - physical pixel dimensions, 32x8 high pixels
    //const char *pathname = "PngSuite/cdsn2c08.png"; // - physical pixel dimensions, 8x8 square pixels
    //const char *pathname = "PngSuite/cdun2c08.png"; // - physical pixel dimensions, 1000 pixels per 1 meter
    //const char *pathname = "PngSuite/ch1n3p04.png"; // - histogram 15 colors
    //const char *pathname = "PngSuite/ch2n3p08.png"; // - histogram 256 colors
    //const char *pathname = "PngSuite/cm0n0g04.png"; // - modification time, 01-jan-2000 12:34:56
    //const char *pathname = "PngSuite/cm7n0g04.png"; // - modification time, 01-jan-1970 00:00:00
    //const char *pathname = "PngSuite/cm9n0g04.png"; // - modification time, 31-dec-1999 23:59:59
    //const char *pathname = "PngSuite/cs3n2c16.png"; // - color, 13 significant bits
    //const char *pathname = "PngSuite/cs3n3p08.png"; // - paletted, 3 significant bits
    //const char *pathname = "PngSuite/cs5n2c08.png"; // - color, 5 significant bits
    //const char *pathname = "PngSuite/cs5n3p08.png"; // - paletted, 5 significant bits
    //const char *pathname = "PngSuite/cs8n2c08.png"; // - color, 8 significant bits (reference)
    //const char *pathname = "PngSuite/cs8n3p08.png"; // - paletted, 8 significant bits (reference)
    //const char *pathname = "PngSuite/ct0n0g04.png"; // - no textual data
    //const char *pathname = "PngSuite/ct1n0g04.png"; // - with textual data
    //const char *pathname = "PngSuite/ctzn0g04.png"; // - with compressed textual data
    //const char *pathname = "PngSuite/cten0g04.png"; // - international UTF-8, english
    //const char *pathname = "PngSuite/ctfn0g04.png"; // - international UTF-8, finnish
    //const char *pathname = "PngSuite/ctgn0g04.png"; // - international UTF-8, greek
    //const char *pathname = "PngSuite/cthn0g04.png"; // - international UTF-8, hindi
    //const char *pathname = "PngSuite/ctjn0g04.png"; // - international UTF-8, japanese

    // Chunk ordering
    //const char *pathname = "PngSuite/oi1n0g16.png"; // - grayscale mother image with 1 idat-chunk
    //const char *pathname = "PngSuite/oi1n2c16.png"; // - color mother image with 1 idat-chunk
    //const char *pathname = "PngSuite/oi2n0g16.png"; // - grayscale image with 2 idat-chunks
    //const char *pathname = "PngSuite/oi2n2c16.png"; // - color image with 2 idat-chunks
    //const char *pathname = "PngSuite/oi4n0g16.png"; // - grayscale image with 4 unequal sized idat-chunks
    //const char *pathname = "PngSuite/oi4n2c16.png"; // - color image with 4 unequal sized idat-chunks
    //const char *pathname = "PngSuite/oi9n0g16.png"; // - grayscale image with all idat-chunks length one
    //const char *pathname = "PngSuite/oi9n2c16.png"; // - color image with all idat-chunks length one

    // Zlib compression
    //const char *pathname = "PngSuite/z00n2c08.png"; // - color, no interlacing, compression level 0 (none)
    //const char *pathname = "PngSuite/z03n2c08.png"; // - color, no interlacing, compression level 3
    //const char *pathname = "PngSuite/z06n2c08.png"; // - color, no interlacing, compression level 6 (default)
    //const char *pathname = "PngSuite/z09n2c08.png"; // - color, no interlacing, compression level 9 (maximum)

    // Corrupted files
    //const char *pathname = "PngSuite/xs1n0g01.png"; // - signature byte 1 MSBit reset to zero
    //const char *pathname = "PngSuite/xs2n0g01.png"; // - signature byte 2 is a 'Q'
    //const char *pathname = "PngSuite/xs4n0g01.png"; // - signature byte 4 lowercase
    //const char *pathname = "PngSuite/xs7n0g01.png"; // - 7th byte a space instead of control-Z
    //const char *pathname = "PngSuite/xcrn0g04.png"; // - added cr bytes
    //const char *pathname = "PngSuite/xlfn0g04.png"; // - added lf bytes
    //const char *pathname = "PngSuite/xhdn0g08.png"; // - incorrect IHDR checksum
    //const char *pathname = "PngSuite/xc1n0g08.png"; // - color type 1
    //const char *pathname = "PngSuite/xc9n2c08.png"; // - color type 9
    //const char *pathname = "PngSuite/xd0n2c08.png"; // - bit-depth 0
    //const char *pathname = "PngSuite/xd3n2c08.png"; // - bit-depth 3
    //const char *pathname = "PngSuite/xd9n2c08.png"; // - bit-depth 99
    //const char *pathname = "PngSuite/xdtn0g01.png"; // - missing IDAT chunk
    //const char *pathname = "PngSuite/xcsn0g01.png"; // - incorrect IDAT checksum

    int width, height, comp;
    int result = gimage_parsePng(pathname, &width, &height, &comp);
    if (result == GIMAGE_NO_ERROR)
    {
        std::vector<unsigned char> buf(width * height * comp);
        gimage_loadPng(pathname, &buf[0]);

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

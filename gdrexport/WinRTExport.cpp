/*
 * WinRTExport.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "WinRTExport.h"

#include <QPainter>
#include <QBuffer>
#include <qpixmap.h>

namespace {
  template<typename T>
  void write(QBuffer& f, const T t)
  {
    f.write((const char*)&t, sizeof(t));
  }
}

void WinRTExport::updateWinRTProject(QString projfile,ExportContext *ctx)
{
    // ----------------------------------------------------------------------
    // For WinRT, write all asset filenames into giderosgame.Windows.vcxproj
    // ----------------------------------------------------------------------
   	ctx->outputDir.cdUp();

      QString replacement;
      for (int i = 0; i < ctx->allfiles.size(); i++){
        QString assetfile=ctx->allfiles[i];
        QString suffix = QFileInfo(assetfile).suffix().toLower();
        //		    outputWidget_->insertPlainText(assetfile);
        //		    outputWidget_->insertPlainText(suffix);

        QString type;
        if (suffix=="jpg" || suffix=="jpeg" || suffix=="png")
          type="Image";
        else if (suffix=="wav" || suffix=="mp3")
          type="Media";
        else if (suffix=="txt")
          type="Text";
        else if (suffix=="ttf")
          type="Font";
        else
          type="None";

        if (type=="None")
          replacement += "<None Include=\"Assets\\"+assetfile+"\">\r\n<DeploymentContent>true</DeploymentContent>\r\n</None>\r\n";
        else
          replacement += "<"+type+" Include=\"Assets\\"+assetfile+"\" />\r\n";
      }

      QByteArray data;
      QFile in(ctx->outputDir.absoluteFilePath(projfile));
      in.open(QFile::ReadOnly);

      data = in.readAll();
      in.close();

      data.replace("<!--GIDEROS_INSERT_ASSETS_HERE-->",replacement.toUtf8());

      QFile out(ctx->outputDir.absoluteFilePath(projfile));
      out.open(QFile::WriteOnly);

      out.write(data);
      out.close();

      ctx->outputDir.cd("Assets");
}

QByteArray WinRTExport::makeIconFile(QImage *image, QColor fill, int mode) {
        int width = 256;
        int height = 256;
        int iwidth = image->width(); //image width
        int iheight = image->height(); //image height
        int rwidth = width; //resampled width
        int rheight = height; //resampled height

        float k_w = fabs((float) width / (float) iwidth); //width scaling coef
        float k_h = fabs((float) height / (float) iheight); //height scaling coef
        int dst_x = 0;
        int dst_y = 0;
        bool redraw = false;
        int sx = 0;
        int sy = 0;
        int sw = -1;
        int sh = -1;

        if (mode == 0) {  //show all
            //use smallest
            if (k_h < k_w) {
                rwidth = round((iwidth * height) / iheight);
            } else {
                rheight = round((iheight * width) / iwidth);
            }

            //new width is bigger than existing
            if (width > rwidth) {
                dst_x = (width - rwidth) / 2;
                redraw = true;
            }

            //new height is bigger than existing
            if (height > rheight) {
                dst_y = (height - rheight) / 2;
                redraw = true;
            }
        } else if (mode == 1) {  //crop
            if (k_h < k_w) {
                rheight = round((iheight * width) / iwidth);
            } else {
                rwidth = round((iwidth * height) / iheight);
            }

            if (width < rwidth) {
                sx = (rwidth - width) / 2;
                sw = width;
                redraw = true;
            }

            if (height < rheight) {
                sy = (rheight - height) / 2;
                sh = height;
                redraw = true;
            }
        }

        QImage xform = image->scaled(rwidth, rheight, Qt::KeepAspectRatio,
                Qt::SmoothTransformation);
        if (redraw)  //(dst_x || dst_y)
        {
            QImage larger(width, height, QImage::Format_ARGB32);
            larger.fill(fill);
            QPainter painter(&larger);
            painter.drawImage(dst_x, dst_y, xform, sx, sy, sw, sh);
            painter.end();
            xform = larger;
        }

        int nsizes=4;
        int sizes[]={256,64,32,16};
        QBuffer f;
        f.open(QFile::OpenModeFlag::WriteOnly);

        // Header
        write<qint16>(f, 0);
        write<qint16>(f, 1);
        write<qint16>(f, nsizes);

        QList<QByteArray> imgs;
        for (int ii = 0; ii < nsizes; ++ii) {
            QBuffer b;
            b.open(QFile::OpenModeFlag::WriteOnly);
            if (xform.width()!=sizes[ii]) {
                xform = xform.scaled(sizes[ii], sizes[ii], Qt::KeepAspectRatio,
                        Qt::SmoothTransformation);
            }
            xform.save(&b,"PNG",0);
            b.close();
            imgs << b.buffer();
        }

        // Images directory
        unsigned int offset = 3 * sizeof(qint16) + nsizes * 16;
        for (int ii = 0; ii < nsizes; ++ii) {
          const auto& pixmap = imgs[ii];
          int nsz=sizes[ii];
          write<char>(f, nsz == 256 ? 0 : nsz);
          write<char>(f, nsz == 256 ? 0 : nsz);
          write<char>(f, 0); // palette size
          write<char>(f, 0); // reserved
          write<qint16>(f, 1); // color planes
          write<qint16>(f, 24); // bits-per-pixel
          write<quint32>(f, pixmap.size()); // size of image in bytes
          write<quint32>(f, offset); // offset
          offset += pixmap.size();
        }

        for (int ii = 0; ii < nsizes; ++ii) {
          const auto& pixmap = imgs[ii];
          f.write(pixmap);
        }

        return f.buffer();
}

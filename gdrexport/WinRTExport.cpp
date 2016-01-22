/*
 * WinRTExport.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "WinRTExport.h"

void WinRTExport::updateWinRTProject(QString projfile,ExportContext *ctx)
{
    // ----------------------------------------------------------------------
    // For WinRT, write all asset filenames into giderosgame.Windows.vcxproj
    // ----------------------------------------------------------------------
   	ctx->outputDir.cdUp();

      QByteArray replacement;
      for (int i = 0; i < ctx->assetfiles.size(); i++){
        QString assetfile=ctx->assetfiles[i];
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

      data.replace("INSERT_ASSETS_HERE",replacement);

      QFile out(ctx->outputDir.absoluteFilePath(projfile));
      out.open(QFile::WriteOnly);

      out.write(data);
      out.close();

      ctx->outputDir.cd("Assets");
}

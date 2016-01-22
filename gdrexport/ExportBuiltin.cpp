/*
 * ExportBuiltin.cpp
 *
 *  Created on: 22 janv. 2016
 *      Author: Nico
 */

#include "ExportBuiltin.h"
#include "ExportCommon.h"
#include "GAppFormat.h"
#include "WinRTExport.h"
#include "MacOSXExport.h"
#include "Utilities.h"
#include <bytebuffer.h>
#include <QFile>

void ExportBuiltin::exportAllAssetsFiles(ExportContext *ctx)
{
    if (ctx->deviceFamily == e_Android) //Configure Jet files
    	ctx->jetset << "mp3" << "mp4" << "png" << "jpg" << "jpeg" << "wav";

	ExportCommon::exportAssets(ctx,ctx->deviceFamily != e_iOS && ctx->deviceFamily != e_MacOSXDesktop);
    if(ctx->deviceFamily == e_MacOSXDesktop || ctx->deviceFamily == e_WindowsDesktop)
        ctx->outputDir.cd("..");

    // write allfiles.txt
    if (ctx->deviceFamily == e_Android)
    	ExportCommon::exportAllfilesTxt(ctx);

    // write luafiles.txt
    ExportCommon::exportLuafilesTxt(ctx);

    // write properties.bin
    ExportCommon::exportPropertiesBin(ctx);
}

void ExportBuiltin::fillTargetReplacements(ExportContext *ctx)
{
    QStringList wildcards1;
    wildcards1 <<
        "*.pch" <<
        "*.plist" <<
        "*.pbxproj" <<
        "*.java" <<
        "*.xml" <<
        "*.appxmanifest" <<
        "*.gradle" <<
        "*.html" <<
        "*.project";
    ctx->wildcards << wildcards1;

    QList<QPair<QByteArray, QByteArray> > replaceList1;
    if (!ctx->templatename.isEmpty())
    {
    	replaceList1 << qMakePair(ctx->templatename.toUtf8(), ctx->base.toUtf8());
    	replaceList1 << qMakePair(ctx->templatenamews.toLatin1(), ctx->basews.toLatin1());
    }
    if (ctx->deviceFamily == e_Android){
        replaceList1 << qMakePair(QString("com.giderosmobile.androidtemplate").toUtf8(), ctx->args["package"].toUtf8());
        replaceList1 << qMakePair(QString("android:versionCode=\"1\"").toUtf8(), ("android:versionCode=\""+QString::number(ctx->properties.version_code)+"\"").toUtf8());
        replaceList1 << qMakePair(QString("android:versionName=\"1.0\"").toUtf8(), ("android:versionName=\""+ctx->properties.version+"\"").toUtf8());
        QString orientation = "android:screenOrientation=\"portrait\"";
        switch(ctx->properties.orientation){
            case 0:
                if(ctx->properties.autorotation > 0)
                    orientation = "android:screenOrientation=\"sensorPortrait\"";
                else
                    orientation = "android:screenOrientation=\"portrait\"";
                break;
            case 1:
                if(ctx->properties.autorotation > 0)
                    orientation = "android:screenOrientation=\"sensorLandscape\"";
                else
                    orientation = "android:screenOrientation=\"landscape\"";
                break;
            case 2:
                if(ctx->properties.autorotation > 0)
                    orientation = "android:screenOrientation=\"sensorPortrait\"";
                else
                    orientation = "android:screenOrientation=\"reversePortrait\"";
                break;
            case 3:
                if(ctx->properties.autorotation > 0)
                    orientation = "android:screenOrientation=\"sensorLandscape\"";
                else
                    orientation = "android:screenOrientation=\"reverseLandscape\"";
                break;
        }

        replaceList1 << qMakePair(QString("android:screenOrientation=\"portrait\"").toUtf8(), orientation.toUtf8());
    }
    else if(ctx->deviceFamily == e_MacOSXDesktop){
        QString category = "public.app-category.games";
        if(ctx->args.contains("category"))
            category = ctx->args["category"];
        if(ctx->args.contains("bundle"))
            replaceList1 << qMakePair(QString("com.yourcompany."+ctx->base).toUtf8(), ctx->args["bundle"].toUtf8());
        replaceList1 << qMakePair(QString("<key>NOTE</key>").toUtf8(), ("<key>LSApplicationCategoryType</key>\n	<string>"+category.toUtf8()+"</string>\n	<key>CFBundleShortVersionString</key>\n	<string>"+ctx->properties.version+"</string>\n	<key>CFBundleVersion</key>\n	<string>"+ctx->properties.version+"</string>\n	<key>CFBundleName</key>\n	<string>"+ctx->base.toUtf8()+"</string>\n	<key>NOTE</key>").toUtf8());
    }
    else if(ctx->deviceFamily == e_iOS){
        if(ctx->args.contains("bundle"))
            replaceList1 << qMakePair(QString("com.yourcompany.${PRODUCT_NAME:rfc1034identifier}").toUtf8(), ctx->args["bundle"].toUtf8());
        replaceList1 << qMakePair(QString("<string>1.0</string>").toUtf8(), ("<string>"+ctx->properties.version+"</string>").toUtf8());
    }
    else if(ctx->deviceFamily == e_WinRT){
        replaceList1 << qMakePair(QString("Gideros Player").toUtf8(), ctx->base.toUtf8());
        replaceList1 << qMakePair(QString("giderosgame").toUtf8(), ctx->basews.toUtf8());
        replaceList1 << qMakePair(QString("com.giderosmobile.windowsphone").toUtf8(), ctx->args["package"].toUtf8());
        replaceList1 << qMakePair(QString("com.giderosmobile.windows").toUtf8(), ctx->args["package"].toUtf8());
        replaceList1 << qMakePair(QString("Gideros Mobile").toUtf8(), ctx->args["organization"].toUtf8());
    }
    else if(ctx->deviceFamily == e_Html5){
        replaceList1 << qMakePair(QString("<title>Gideros</title>").toUtf8(), ("<title>"+ctx->base+"</title>").toUtf8());
        replaceList1 << qMakePair(QString("gideros.GApp").toUtf8(), (ctx->base+".GApp").toUtf8());
    }
    ctx->replaceList << replaceList1;
}

void ExportBuiltin::prepareAssetFolder(ExportContext *ctx)
{
    if (ctx->deviceFamily == e_iOS)
    {
    	ctx->outputDir.mkdir(ctx->base);
    	ctx->outputDir.cd(ctx->base);
    }
    else if (ctx->deviceFamily == e_Android)
    {
        if(ctx->args["template"] == "androidstudio"){
        	ctx->outputDir.cd("app");
        	ctx->outputDir.cd("src");
        	ctx->outputDir.cd("main");
        	ctx->outputDir.mkdir("assets");
        	ctx->outputDir.cd("assets");
        }
        else{
        	ctx->outputDir.mkdir("assets");
        	ctx->outputDir.cd("assets");
        }
    }
    else if(ctx->deviceFamily == e_MacOSXDesktop)
    {
    	ctx->outputDir.cd(ctx->base + ".app");
    	ctx->outputDir.cd("Contents");
    }
    else if (ctx->deviceFamily == e_WinRT)
    {
    	ctx->outputDir.cd("giderosgame");
    	ctx->outputDir.cd("giderosgame.Windows");
    	ctx->outputDir.cd("Assets");
    }

    if (ctx->deviceFamily != e_WinRT){
    	ctx->outputDir.mkdir("assets");
    	ctx->outputDir.cd("assets");
    }

        if(ctx->deviceFamily == e_MacOSXDesktop || ctx->deviceFamily == e_WindowsDesktop){
            QString org;
            QString domain;
            if(ctx->deviceFamily == e_MacOSXDesktop){
                org = ctx->args["organization"];
                domain = ctx->args["domain"];
            }
            else if(ctx->deviceFamily == e_WindowsDesktop){
                org = ctx->args["organization"];
                domain = ctx->args["domain"];
            }
            QString filename = "data.bin";
            QFile file(QDir::cleanPath(ctx->outputDir.absoluteFilePath(filename)));
            if (file.open(QIODevice::WriteOnly))
            {
                ByteBuffer buffer;

                buffer << org.toStdString().c_str();
                buffer << domain.toStdString().c_str();
                buffer << ctx->base.toStdString().c_str();

                file.write(buffer.data(), buffer.size());
            }
            ctx->outputDir.mkdir("resource");
            ctx->outputDir.cd("resource");
        }


        for (std::size_t i = 0; i < ctx->folderList.size(); ++i){
        	ctx->outputDir.mkdir(ctx->folderList[i]);
            if (ctx->deviceFamily == e_WinRT){
            	ctx->outputDir.cdUp();
            	ctx->outputDir.cdUp();
            	ctx->outputDir.cd("giderosgame.WindowsPhone");
            	ctx->outputDir.cd("Assets");

            	ctx->outputDir.mkdir(ctx->folderList[i]);

            	ctx->outputDir.cdUp();
            	ctx->outputDir.cdUp();
            	ctx->outputDir.cd("giderosgame.Windows");
            	ctx->outputDir.cd("Assets");
            }
        }
}

void ExportBuiltin::doExport(ExportContext *ctx)
{
    QString templatedir;
    bool underscore=false;
    bool needGApp=false;

    switch (ctx->deviceFamily)
    {
    case e_iOS:
      templatedir = "Xcode4";
      ctx->templatename = "iOS Template";
      ctx->templatenamews = "iOS_Template";
      underscore = true;
      break;

    case e_Android:
      templatedir = "Eclipse";
      if(ctx->args.contains("template") && ctx->args["template"] == "androidstudio")
          templatedir = "AndroidStudio";
      ctx->templatename = "Android Template";
      ctx->templatenamews = "AndroidTemplate";
      underscore = false;
      break;

    case e_WinRT:
      templatedir = "VisualStudio";
      ctx->templatename = "WinRT Template";
      ctx->templatenamews = "WinRTTemplate";
      underscore = true;
      break;

    case e_Win32:
      templatedir = "win32";
      ctx->templatename = "WindowsDesktopTemplate";
      ctx->templatenamews = "WindowsDesktopTemplate";
      underscore = true;
      break;

    case e_WindowsDesktop:
        templatedir = "Qt";
        ctx->templatename = "WindowsDesktopTemplate";
        ctx->templatenamews = "WindowsDesktopTemplate";
        underscore = false;
        break;

    case e_MacOSXDesktop:
        templatedir = "Qt";
        ctx->templatename = "MacOSXDesktopTemplate";
        ctx->templatenamews = "MacOSXDesktopTemplate";
        underscore = false;
        break;
    case e_GApp:
        underscore = false;
        needGApp = true;
        break;
    case e_Html5:
    	templatedir = "Html5";
    	ctx->templatename = "Html5";
    	ctx->templatenamews = "Html5";
        underscore = false;
        needGApp = true;
        break;
    }

    ctx->basews=Utilities::RemoveSpaces(ctx->base,underscore);
    ExportBuiltin::fillTargetReplacements(ctx);

   // copy template
   if (templatedir.length()>0)
   	ExportCommon::copyTemplate(QString("Templates").append("/").append(templatedir).append("/").append(ctx->templatename),ctx);

   ExportBuiltin::prepareAssetFolder(ctx);
   ExportBuiltin::exportAllAssetsFiles(ctx);
   if (ctx->deviceFamily == e_WinRT)
   {
   	WinRTExport::updateWinRTProject(QString("giderosgame.Windows.vcxproj"),ctx);
       ctx->outputDir.cdUp();
       ctx->outputDir.cdUp();
       ctx->outputDir.cd("giderosgame.WindowsPhone");
       ctx->outputDir.cd("Assets");
       ExportBuiltin::exportAllAssetsFiles(ctx);
       WinRTExport::updateWinRTProject(QString("giderosgame.WindowsPhone.vcxproj"),ctx);
   }

   if (needGApp)
   {
   	ctx->outputDir.cdUp();
       if (ctx->deviceFamily == e_GApp)
    	   ctx->outputDir.cdUp();
       GAppFormat::buildGApp(QDir::cleanPath(ctx->outputDir.absoluteFilePath(ctx->base+".GApp")),ctx);
       if (ctx->deviceFamily == e_GApp)
       	ctx->outputDir.cd(ctx->base);
       else
       	ctx->outputDir.cd("assets");
       ctx->outputDir.removeRecursively();
       ctx->outputDir.cdUp();
   }

#ifdef Q_OS_MACX
   if(ctx->deviceFamily == e_MacOSXDesktop){
   	MacOSXExport::CodeSignMacOSX(&ctx);
   }
#endif

}


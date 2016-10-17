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

	ExportCommon::exportAssets(ctx,true);
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
        if(ctx->properties.disableSplash)
            replaceList1 << qMakePair(QString("boolean showSplash = true;").toUtf8(), QString("boolean showSplash = false;").toUtf8());

        replaceList1 << qMakePair(QString("Color.parseColor(\"#ffffff\")").toUtf8(), QString("Color.parseColor(\""+ctx->properties.backgroundColor+"\")").toUtf8());
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
        replaceList1 << qMakePair(QString("BackgroundColor=\"#464646\"").toUtf8(), ("BackgroundColor=\""+ctx->properties.backgroundColor+"\"").toUtf8());
        replaceList1 << qMakePair(QString("BackgroundColor=\"transparent\"").toUtf8(), ("BackgroundColor=\""+ctx->properties.backgroundColor+"\"").toUtf8());
    }
    else if(ctx->deviceFamily == e_Html5){
        replaceList1 << qMakePair(QString("<title>Gideros</title>").toUtf8(), ("<title>"+ctx->base+"</title>").toUtf8());
        replaceList1 << qMakePair(QString("<body class=\"fullscreen\">").toUtf8(), ("<body class=\"fullscreen\" style=\"background-color:"+ctx->properties.backgroundColor+";\">").toUtf8());
        if(ctx->properties.disableSplash)
            replaceList1 << qMakePair(QString("<img src=\"gideros.png\" />").toUtf8(), QString("<img src=\"gideros.png\" style=\"display:none;\"/>").toUtf8());
        replaceList1 << qMakePair(QString("gideros.GApp").toUtf8(), (ctx->base+".GApp").toUtf8());
        replaceList1 << qMakePair(QString("GIDEROS_MEMORY_MB=128").toUtf8(),QString("GIDEROS_MEMORY_MB=%1").arg(ctx->properties.html5_mem).toUtf8());
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
      ctx->platform = "iOS";
      underscore = true;
      break;

    case e_Android:
      templatedir = "Eclipse";
      ctx->platform = "Android";
      if(ctx->args.contains("template") && ctx->args["template"] == "androidstudio")
      {
          templatedir = "AndroidStudio";
          ctx->platform = "AndroidStudio";
      }
      ctx->templatename = "Android Template";
      ctx->templatenamews = "AndroidTemplate";
      underscore = false;
      break;

    case e_WinRT:
      templatedir = "VisualStudio";
      ctx->templatename = "WinRT Template";
      ctx->templatenamews = "WinRTTemplate";
      ctx->platform = "WinRT";
      underscore = true;
      break;

    case e_Win32:
      templatedir = "win32";
      ctx->templatename = "WindowsDesktopTemplate";
      ctx->templatenamews = "WindowsDesktopTemplate";
      ctx->platform = "Win32";
      underscore = true;
      break;

    case e_WindowsDesktop:
        templatedir = "Qt";
        ctx->templatename = "WindowsDesktopTemplate";
        ctx->templatenamews = "WindowsDesktopTemplate";
        ctx->platform = "WindowsDesktop";
        underscore = false;
        break;

    case e_MacOSXDesktop:
        templatedir = "Qt";
        ctx->templatename = "MacOSXDesktopTemplate";
        ctx->templatenamews = "MacOSXDesktopTemplate";
        ctx->platform = "MacOSXDesktop";
        underscore = false;
        break;
    case e_GApp:
        underscore = false;
        needGApp = true;
        ctx->platform = "GApp";
        break;
    case e_Html5:
    	templatedir = "Html5";
    	ctx->templatename = "Html5";
    	ctx->templatenamews = "Html5";
        ctx->platform = "Html5";
        underscore = false;
        needGApp = true;
        break;
    }

    ctx->basews=Utilities::RemoveSpaces(ctx->base,underscore);
    ExportBuiltin::fillTargetReplacements(ctx);

   // copy template
   if (templatedir.length()>0)
    ExportCommon::copyTemplate(QString("Templates").append("/").append(templatedir).append("/").append(ctx->templatename),"",ctx, false, QStringList(), QStringList());

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

   //go back to root
   ctx->outputDir = QDir(ctx->exportDir);

   //install plugins
   ExportCommon::applyPlugins(ctx);

   if (needGApp)
   {
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

   //exporting icons
   if (ctx->deviceFamily == e_Html5)
   {
       ExportCommon::splashHImage(ctx,615,215,QString("gideros.png"));
	   ExportCommon::appIcon(ctx,64,64,QString("favicon.png"));
   }
   else if(ctx->deviceFamily == e_Android){
       if(templatedir.compare("Eclipse") == 0){
           ExportCommon::appIcon(ctx,36,36,QString("res/drawable-ldpi/icon.png"));
           ExportCommon::appIcon(ctx,48,48,QString("res/drawable-mdpi/icon.png"));
           ExportCommon::appIcon(ctx,72,72,QString("res/drawable-hdpi/icon.png"));
           ExportCommon::appIcon(ctx,96,96,QString("res/drawable-xhdpi/icon.png"));
           ExportCommon::appIcon(ctx,144,144,QString("res/drawable-xxhdpi/icon.png"));
           ExportCommon::appIcon(ctx,192,192,QString("res/drawable-xxxhdpi/icon.png"));
           ExportCommon::appIcon(ctx,96,96,QString("res/drawable/app_icon.png"));

           //tv stuff
           ExportCommon::appIcon(ctx,732,412,QString("res/drawable-xhdpi/ouya_icon.png"));
           ExportCommon::appIcon(ctx,320,180,QString("res/drawable/icon.png"));

           if(ctx->properties.orientation == 0 || ctx->properties.orientation == 2){
               ExportCommon::splashVImage(ctx,200,320,QString("res/drawable-ldpi/splash.png"));
               ExportCommon::splashVImage(ctx,320,480,QString("res/drawable-mdpi/splash.png"));
               ExportCommon::splashVImage(ctx,480,800,QString("res/drawable-hdpi/splash.png"));
               ExportCommon::splashVImage(ctx,720,1280,QString("res/drawable-xhdpi/splash.png"));
               ExportCommon::splashVImage(ctx,960,1600,QString("res/drawable-xxhdpi/splash.png"));
               ExportCommon::splashVImage(ctx,1280,1920,QString("res/drawable-xxxhdpi/splash.png"));
           }
           else{
               ExportCommon::splashHImage(ctx,320,200,QString("res/drawable-ldpi/splash.png"));
               ExportCommon::splashHImage(ctx,480,320,QString("res/drawable-mdpi/splash.png"));
               ExportCommon::splashHImage(ctx,800,480,QString("res/drawable-hdpi/splash.png"));
               ExportCommon::splashHImage(ctx,1280,720,QString("res/drawable-xhdpi/splash.png"));
               ExportCommon::splashHImage(ctx,1600,960,QString("res/drawable-xxhdpi/splash.png"));
               ExportCommon::splashHImage(ctx,1920,1280,QString("res/drawable-xxxhdpi/splash.png"));
           }
       }
       else{
           ExportCommon::appIcon(ctx,36,36,QString("app/src/main/res/drawable-ldpi/icon.png"));
           ExportCommon::appIcon(ctx,48,48,QString("app/src/main/res/drawable-mdpi/icon.png"));
           ExportCommon::appIcon(ctx,72,72,QString("app/src/main/res/drawable-hdpi/icon.png"));
           ExportCommon::appIcon(ctx,96,96,QString("app/src/main/res/drawable-xhdpi/icon.png"));
           ExportCommon::appIcon(ctx,144,144,QString("app/src/main/res/drawable-xxhdpi/icon.png"));
           ExportCommon::appIcon(ctx,192,192,QString("app/src/main/res/drawable-xxxhdpi/icon.png"));

           if(ctx->properties.orientation == 0 || ctx->properties.orientation == 2){
               ExportCommon::splashVImage(ctx,200,320,QString("app/src/main/res/drawable-ldpi/splash.png"));
               ExportCommon::splashVImage(ctx,320,480,QString("app/src/main/res/drawable-mdpi/splash.png"));
               ExportCommon::splashVImage(ctx,480,800,QString("app/src/main/res/drawable-hdpi/splash.png"));
               ExportCommon::splashVImage(ctx,720,1280,QString("app/src/main/res/drawable-xhdpi/splash.png"));
               ExportCommon::splashVImage(ctx,960,1600,QString("app/src/main/res/drawable-xxhdpi/splash.png"));
               ExportCommon::splashVImage(ctx,1280,1920,QString("app/src/main/res/drawable-xxxhdpi/splash.png"));
           }
           else{
               ExportCommon::splashHImage(ctx,320,200,QString("app/src/main/res/drawable-ldpi/splash.png"));
               ExportCommon::splashHImage(ctx,480,320,QString("app/src/main/res/drawable-mdpi/splash.png"));
               ExportCommon::splashHImage(ctx,800,480,QString("app/src/main/res/drawable-hdpi/splash.png"));
               ExportCommon::splashHImage(ctx,1280,720,QString("app/src/main/res/drawable-xhdpi/splash.png"));
               ExportCommon::splashHImage(ctx,1600,960,QString("app/src/main/res/drawable-xxhdpi/splash.png"));
               ExportCommon::splashHImage(ctx,1920,1280,QString("app/src/main/res/drawable-xxxhdpi/splash.png"));
           }
       }
   }
   else if(ctx->deviceFamily == e_WinRT){
        ExportCommon::appIcon(ctx,120,120,QString("giderosgame/giderosgame.Windows/Assets/Logo.scale-80.png"));
        ExportCommon::appIcon(ctx,150,150,QString("giderosgame/giderosgame.Windows/Assets/Logo.scale-100.png"));
        ExportCommon::appIcon(ctx,30,30,QString("giderosgame/giderosgame.Windows/Assets/SmallLogo.scale-100.png"));
        ExportCommon::appIcon(ctx,50,50,QString("giderosgame/giderosgame.Windows/Assets/StoreLogo.scale-100.png"));

        ExportCommon::appIcon(ctx,150,150,QString("giderosgame/giderosgame.WindowsPhone/Assets/Logo.scale-100.png"));
        ExportCommon::appIcon(ctx,360,360,QString("giderosgame/giderosgame.WindowsPhone/Assets/Logo.scale-240.png"));
        ExportCommon::appIcon(ctx,44,44,QString("giderosgame/giderosgame.WindowsPhone/Assets/SmallLogo.scale-100.png"));
        ExportCommon::appIcon(ctx,106,106,QString("giderosgame/giderosgame.WindowsPhone/Assets/SmallLogo.scale-240.png"));
        ExportCommon::appIcon(ctx,170,170,QString("giderosgame/giderosgame.WindowsPhone/Assets/Square71x71Logo.scale-240.png"));
        ExportCommon::appIcon(ctx,50,50,QString("giderosgame/giderosgame.WindowsPhone/Assets/StoreLogo.scale-100.png"));
        ExportCommon::appIcon(ctx,120,120,QString("giderosgame/giderosgame.WindowsPhone/Assets/StoreLogo.scale-240.png"));

        ExportCommon::splashHImage(ctx,620,300,QString("giderosgame/giderosgame.Windows/Assets/SplashScreen.scale-100.png"));
        ExportCommon::splashVImage(ctx,480,800,QString("giderosgame/giderosgame.WindowsPhone/Assets/SplashScreen.scale-100.png"));
        ExportCommon::splashVImage(ctx,1152,1920,QString("giderosgame/giderosgame.WindowsPhone/Assets/SplashScreen.scale-240.png"));
        ExportCommon::splashHImage(ctx,744,360,QString("giderosgame/giderosgame.WindowsPhone/Assets/WideLogo.scale-240.png"));
   }
   else if(ctx->deviceFamily == e_iOS){
        ExportCommon::appIcon(ctx,29,29,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon29x29.png"));
        ExportCommon::appIcon(ctx,40,40,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon40x40.png"));
        ExportCommon::appIcon(ctx,50,50,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon50x50.png"));
        ExportCommon::appIcon(ctx,57,57,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon57x57.png"));
        ExportCommon::appIcon(ctx,58,58,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon58x58.png"));
        ExportCommon::appIcon(ctx,72,72,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon72x72.png"));
        ExportCommon::appIcon(ctx,76,76,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon76x76.png"));
        ExportCommon::appIcon(ctx,80,80,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon80x80.png"));
        ExportCommon::appIcon(ctx,87,87,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon87x87.png"));
        ExportCommon::appIcon(ctx,100,100,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon100x100.png"));
        ExportCommon::appIcon(ctx,114,114,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon114x114.png"));
        ExportCommon::appIcon(ctx,120,120,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon120x120.png"));
        ExportCommon::appIcon(ctx,144,144,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon144x144.png"));
        ExportCommon::appIcon(ctx,152,152,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon152x152.png"));
        ExportCommon::appIcon(ctx,167,167,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon167x167.png"));
        ExportCommon::appIcon(ctx,180,180,QString(ctx->base+" iOS/Images.xcassets/AppIcon.appiconset/AppIcon180x180.png"));


        ExportCommon::splashHImage(ctx,1024,768,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash1024x768.png"));
        ExportCommon::splashHImage(ctx,1024,748,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash1024x748.png"));
        ExportCommon::splashHImage(ctx,2048,1536,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash2048x1536.png"));
        ExportCommon::splashHImage(ctx,2048,1496,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash2048x1496.png"));
        ExportCommon::splashHImage(ctx,2208,1242,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash2208x1242.png"));

        ExportCommon::splashVImage(ctx,768,1024,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash768x1024.png"));
        ExportCommon::splashVImage(ctx,768,1004,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash768x1004.png"));
        ExportCommon::splashVImage(ctx,1536,2048,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash1536x2048.png"));
        ExportCommon::splashVImage(ctx,1536,2008,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash1536x2008.png"));
        ExportCommon::splashVImage(ctx,640,960,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash640x960.png"));
        ExportCommon::splashVImage(ctx,640,1136,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash640x1136.png"));
        ExportCommon::splashVImage(ctx,750,1334,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash750x1334.png"));
        ExportCommon::splashVImage(ctx,1242,2208,QString(ctx->base+" iOS/Images.xcassets/LaunchImage.launchimage/Splash1242x2208.png"));

        //tv stuff
        ExportCommon::splashHImage(ctx,1920,1080,QString(ctx->base+" iOS/Images.xcassets/TVLaunchImage.launchimage/Splash1920x1080.png"));
        ExportCommon::tvIcon(ctx,1920,720,QString(ctx->base+" iOS/Images.xcassets/App Icon & Top Shelf Image.brandassets/Top Shelf Image.imageset/TVIcon1920x720.png"));
        ExportCommon::tvIcon(ctx,1280,768,QString(ctx->base+" iOS/Images.xcassets/App Icon & Top Shelf Image.brandassets/Top Shelf Image.imageset/TVIcon1280x768.png"));
        ExportCommon::tvIcon(ctx,400,240,QString(ctx->base+" iOS/Images.xcassets/App Icon & Top Shelf Image.brandassets/Top Shelf Image.imageset/TVIcon400x240.png"));
   }

#ifdef Q_OS_MACX
    if(ctx->deviceFamily == e_MacOSXDesktop){
        MacOSXExport::CodeSignMacOSX(ctx);
    }
#endif

}


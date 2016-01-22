#include <QDir>
#include <QSet>
#include <QString>
#include <QTextStream>
#include <QSettings>
#include <time.h>
#include <QDomDocument>
#include <projectproperties.h>
#include <orientation.h>
#include <stack>
#include <dependencygraph.h>
#include <QProcess>
#include <bytebuffer.h>
#include <QCoreApplication>
#include "exportcontext.h"
#include "GAppFormat.h"
#include "Utilities.h"
#include "WinRTExport.h"
#include "MacOSXExport.h"
#include "ExportCommon.h"

static bool readProjectFile(const QString& fileName,
                            ProjectProperties &properties,
                            std::vector<std::pair<QString, QString> > &fileList,
                            std::vector<QString> &folderList,
                            DependencyGraph &dependencyGraph)
{
    ProjectProperties &properties_ = properties;
    std::vector<std::pair<QString, QString> > &fileList_ = fileList;
    DependencyGraph &dependencyGraph_ = dependencyGraph;

    QDomDocument doc;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
    {
        fprintf(stderr, "Cannot open project file: %s\n", qPrintable(fileName));
        return false;
    }

    if (!doc.setContent(&file))
    {
        fprintf(stderr, "Cannot parse project file: %s\n", qPrintable(fileName));
        file.close();
        return false;
    }

    file.close();

    QDomElement root = doc.documentElement();

    // read properties
    {
        properties_.clear();

        QDomElement properties = root.firstChildElement("properties");
        properties_.loadXml(properties);
    }


    // populate file list, folder list and dependency graph
    {
        fileList_.clear();
        dependencyGraph_.clear();
        std::vector<std::pair<QString, QString> > dependencies_;

        std::stack<QDomNode> stack;
        stack.push(doc.documentElement());

        std::vector<QString> dir;

        while (stack.empty() == false)
        {
            QDomNode n = stack.top();
            QDomElement e = n.toElement();
            stack.pop();

            if (n.isNull() == true)
            {
                dir.pop_back();
                continue;
            }

            QString type = e.tagName();

            if (type == "file")
            {
                QString fileName = e.hasAttribute("source") ? e.attribute("source") : e.attribute("file");
                QString name = QFileInfo(fileName).fileName();

                QString n;
                for (std::size_t i = 0; i < dir.size(); ++i)
                    n += dir[i] + "/";
                n += name;

                fileList_.push_back(std::make_pair(n, fileName));

                if (QFileInfo(fileName).suffix().toLower() == "lua")
                {
                    bool excludeFromExecution = e.hasAttribute("excludeFromExecution") && e.attribute("excludeFromExecution").toInt();
                    dependencyGraph_.addCode(fileName, excludeFromExecution);
                }

                continue;
            }

            if (type == "folder")
            {
                QString name = e.attribute("name");
                dir.push_back(name);

                QString n;
                for (std::size_t i = 0; i < dir.size(); ++i)
                    n += dir[i] + "/";

                folderList.push_back(n);

                stack.push(QDomNode());
            }

            if (type == "dependency")
            {
                QString from = e.attribute("from");
                QString to = e.attribute("to");

                dependencies_.push_back(std::make_pair(from, to));
            }

            QDomNodeList childNodes = n.childNodes();
            for (int i = 0; i < childNodes.size(); ++i)
                stack.push(childNodes.item(i));
        }

        for (size_t i = 0; i < dependencies_.size(); ++i)
            dependencyGraph_.addDependency(dependencies_[i].first,dependencies_[i].second);
    }

    return true;
}

void usage()
{
    fprintf(stderr, "Usage: gdrexport -options <project_file> <output_dir>\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options general: \n");
    fprintf(stderr, "    -platform <platform_name>  #platform to export (ios, android, windows, macosx, winrt, win32, gapp, html5)\n");
    fprintf(stderr, "    -encrypt                   #encrypts code and assets\n");
    fprintf(stderr, "    -encrypt-code              #encrypts code\n");
    fprintf(stderr, "    -encrypt-assets            #encrypts assets\n");
    fprintf(stderr, "    -assets-only               #exports only assets\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options ios: \n");
    fprintf(stderr, "    -bundle <bundle_id>        #bundle id\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options android: \n");
    fprintf(stderr, "    -package <package_name>    #apk package name\n");
    fprintf(stderr, "    -template <template>       #template to use (eclipse, androidstudio)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options windows: \n");
    fprintf(stderr, "    -organization <name>       #organization name\n");
    fprintf(stderr, "    -domain <domain_name>      #domain name\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options macosx: \n");
    fprintf(stderr, "    -organization <name>       #organization name\n");
    fprintf(stderr, "    -domain <domain_name>      #domain name\n");
    fprintf(stderr, "    -bundle <bundle_id>        #bundle id\n");
    fprintf(stderr, "    -category <app category>   #category of your app\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options winrt: \n");
    fprintf(stderr, "    -organization <name>       #organization name\n");
    fprintf(stderr, "    -package <package_name>    #package name\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Options html5: \n");
    fprintf(stderr, "    -hostname <name>           #host name the app will be run from\n");
}

void fillTargetReplacements(ExportContext *ctx)
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

void prepareAssetFolder(ExportContext *ctx)
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

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("GiderosMobile");
    QCoreApplication::setOrganizationDomain("giderosmobile.com");
    QCoreApplication::setApplicationName("GiderosStudio");

    QSettings::setDefaultFormat(QSettings::IniFormat);

    QCoreApplication a(argc, argv);

    QStringList arguments = a.arguments();

    ExportContext ctx;
    ctx.deviceFamily=e_None;
    QString projectFileName;
    QString output;
    bool encryptCode = false;
    bool encryptAssets = false;
    bool assetsOnly = false;

    //Lookup XML exports scripts
    QMap<QString,QString> xmlExports;
    {
    QDir sourceDir("Templates");
   	QStringList filters;
    filters << "*.gexport";
    sourceDir.setNameFilters(filters);
    QStringList files = sourceDir.entryList(QDir::Files | QDir::Hidden);
    for(int i = 0; i < files.count(); i++)
    {
	       QDomDocument doc("export");
	       QFile file(sourceDir.absoluteFilePath(files[i]));
           if (!file.open(QIODevice::ReadOnly))
                continue;
            if (!doc.setContent(&file))
            {
                file.close();
                continue;
            }
            file.close();

            QDomElement exporter = doc.documentElement();
            QString exname=exporter.attribute("name");
            xmlExports[exname]=sourceDir.absoluteFilePath(files[i]);
	}
    }


    int i = 1;
    while (i < arguments.size())
    {
        if (arguments[i] == "-platform")
        {
            if (i + 1 >= arguments.size())
            {
                fprintf(stderr, "Missing argument: platform_name\n\n");
                usage();
                return 1;
            }
            QString platform = arguments[i + 1];
            ctx.platform=platform;
            if (platform.toLower() == "ios")
            {
                ctx.deviceFamily = e_iOS;
            }
            else if (platform.toLower() == "android")
            {
            	ctx.deviceFamily = e_Android;
            }
            else if (platform.toLower() == "windows")
            {
            	ctx.deviceFamily = e_WindowsDesktop;
            }
            else if (platform.toLower() == "macosx")
            {
            	ctx.deviceFamily = e_MacOSXDesktop;
            }
            else if (platform.toLower() == "winrt")
            {
            	ctx.deviceFamily = e_WinRT;
            }
            else if (platform.toLower() == "win32")
            {
            	ctx.deviceFamily = e_Win32;
            }
            else if (platform.toLower() == "gapp")
            {
            	ctx.deviceFamily = e_GApp;
            }
            else if (platform.toLower() == "html5")
            {
            	ctx.deviceFamily = e_Html5;
            }
            else if (xmlExports.contains(platform))
            {
            	ctx.deviceFamily = e_Xml;
            }
            else
            {
                fprintf(stderr, "Unknown argument for platform_name: %s\n\n", qPrintable(platform));
                usage();
                return 1;
            }
            i += 2;
        }
        else if (arguments[i] == "-encrypt")
        {
            encryptCode = true;
            encryptAssets = true;
            i++;
        }
        else if (arguments[i] == "-encrypt-code")
        {
            encryptCode = true;
            i++;
        }
        else if (arguments[i] == "-encrypt-assets")
        {
            encryptAssets = true;
            i++;
        }
        else if (arguments[i] == "-assets-only")
        {
            assetsOnly = true;
            i++;
        }
        else if (arguments[i].startsWith("-"))
        {
            if (i + 1 >= arguments.size())
            {
                fprintf(stderr, "Missing value fo argument: %s\n\n", arguments[i].toStdString().c_str());
                usage();
                return 1;
            }
            ctx.args.insert(arguments[i].remove(0,1), arguments[i + 1]);
            i += 2;
        }
        else
        {
            if (projectFileName.isEmpty())
            {
                projectFileName = arguments[i];
                i++;
            }
            else if (output.isEmpty())
            {
                output = arguments[i];
                i++;
            }
            else
            {
                fprintf(stderr, "Unknown argument: %s\n\n", qPrintable(arguments[i]));
                usage();
                return 1;
            }
        }
    }

    if (projectFileName.isEmpty())
    {
        fprintf(stderr, "Missing argument: project_file\n\n");
        usage();
        return 1;
    }

    if (output.isEmpty())
    {
        fprintf(stderr, "Missing argument: output_dir\n\n");
        usage();
        return 1;
    }

    if (ctx.deviceFamily == e_None)
    {
        fprintf(stderr, "Missing argument: platform_name\n\n");
        usage();
        return 1;
    }

    QDir outputDir(output);

    projectFileName = QDir::current().absoluteFilePath(projectFileName);

    QDir cdir = QCoreApplication::applicationDirPath();
    cdir.cdUp();
    QDir::setCurrent(cdir.absolutePath());

    QString templatedir;
    bool underscore=false;
    bool needGApp=false;

    switch (ctx.deviceFamily)
    {
    case e_iOS:
      templatedir = "Xcode4";
      ctx.templatename = "iOS Template";
      ctx.templatenamews = "iOS_Template";
      underscore = true;
      break;

    case e_Android:
      templatedir = "Eclipse";
      if(ctx.args.contains("template") && ctx.args["template"] == "androidstudio")
          templatedir = "AndroidStudio";
      ctx.templatename = "Android Template";
      ctx.templatenamews = "AndroidTemplate";
      underscore = false;
      break;

    case e_WinRT:
      templatedir = "VisualStudio";
      ctx.templatename = "WinRT Template";
      ctx.templatenamews = "WinRTTemplate";
      underscore = true;
      break;

    case e_Win32:
      templatedir = "win32";
      ctx.templatename = "WindowsDesktopTemplate";
      ctx.templatenamews = "WindowsDesktopTemplate";
      underscore = true;
      break;

    case e_WindowsDesktop:
        templatedir = "Qt";
        ctx.templatename = "WindowsDesktopTemplate";
        ctx.templatenamews = "WindowsDesktopTemplate";
        underscore = false;
        break;

    case e_MacOSXDesktop:
        templatedir = "Qt";
        ctx.templatename = "MacOSXDesktopTemplate";
        ctx.templatenamews = "MacOSXDesktopTemplate";
        underscore = false;
        break;
    case e_GApp:
        underscore = false;
        needGApp = true;
        break;
    case e_Html5:
    	templatedir = "Html5";
    	ctx.templatename = "Html5";
    	ctx.templatenamews = "Html5";
        underscore = false;
        needGApp = true;
        break;
    }

    const QString &projectFileName_ = projectFileName;
    ctx.projectFileName_=projectFileName_;
    ctx.base = QFileInfo(projectFileName_).baseName();
    ctx.basews=Utilities::RemoveSpaces(ctx.base,underscore);

    outputDir.mkdir(ctx.base);
    outputDir.cd(ctx.base);
    ctx.outputDir=outputDir;

    QByteArray codePrefix("312e68c04c6fd22922b5b232ea6fb3e1");
    QByteArray assetsPrefix("312e68c04c6fd22922b5b232ea6fb3e2");
    QByteArray codePrefixRnd("312e68c04c6fd22922b5b232ea6fb3e1");
    QByteArray assetsPrefixRnd("312e68c04c6fd22922b5b232ea6fb3e2");
    QByteArray encryptionZero(256, '\0');
    QByteArray codeKey(256, '\0');
    QByteArray assetsKey(256, '\0');
    QByteArray randomData(32+32+256+256, '\0');

    {
        QSettings settings;
        if (settings.contains("randomData"))
        {
            randomData = settings.value("randomData").toByteArray();
        }
        else
        {
            qsrand(time(NULL));
            for (int i = 0; i < randomData.size(); ++i)
                randomData[i] = qrand() % 256;
            settings.setValue("randomData", randomData);
            settings.sync();
        }
    }

    if ((ctx.deviceFamily==e_Html5)&&(!ctx.args["hostname"].isEmpty()))
    {
    	encryptAssets=true;
    	encryptCode=true;
    }

    if (encryptCode)
    {
        codeKey = randomData.mid(64,256);
        codePrefixRnd=randomData.mid(0,32);
    }

    if (encryptAssets)
    {
        assetsKey = randomData.mid(64+256,256);
        assetsPrefixRnd=randomData.mid(32,32);
    }

    if (ctx.deviceFamily==e_Html5)
    {
    	encryptAssets=true;
    	encryptCode=true;
    	if (!(ctx.args["hostname"].isEmpty()))
    	{
    		QByteArray mkey=ctx.args["hostname"].toUtf8();
        	int msize=mkey.size();
        	if (msize>255)
        	{
        		msize=255; //Unlikely to happen
        		mkey.truncate(255);
        	}
        	mkey.append((char)0);
        	msize++;
    		codeKey.replace(0,msize,mkey);
    	}
    	else
    	{
    	    QByteArray zero(1, '\0');
    		codeKey.replace(0,1,zero);
    	}
    }

    DependencyGraph dependencyGraph;
    if (readProjectFile(projectFileName, ctx.properties, ctx.fileQueue, ctx.folderList, dependencyGraph) == false)
    {
        // error is displayed at readProjectFile function
        return 1;
    }
    std::vector<std::pair<QString, bool> > topologicalSort = dependencyGraph.topologicalSort();
     for (std::size_t i = 0; i < topologicalSort.size(); ++i)
     {
         int index = -1;
         for (std::size_t j = 0; j < ctx.fileQueue.size(); ++j)
         {
             if (ctx.fileQueue[j].second == topologicalSort[i].first)
             {
                 index = j;
                 break;
             }
         }

         if (index != -1)
         {
             std::pair<QString, QString> item = ctx.fileQueue[index];
             ctx.fileQueue.erase(ctx.fileQueue.begin() + index);
             ctx.fileQueue.push_back(item);
         }
     }
     ctx.assetsOnly=assetsOnly;
     ctx.codeKey=codeKey;
     ctx.assetsKey=assetsKey;
     ctx.topologicalSort=topologicalSort;

     //Encryption key replacement info
     QStringList wildcards2;
     wildcards2 << "libgideros.so" << "libgideros.a" << "gid.dll"
     		   << "libgid.1.dylib" << "gideros.WindowsPhone.lib"
     		   << "gideros.Windows.lib" << "gideros.html.mem";
     ctx.wildcards << wildcards2;

     fillTargetReplacements(&ctx);

    QList<QPair<QByteArray, QByteArray> > replaceList2;
    replaceList2 << qMakePair(QByteArray("9852564f4728e0c11e34ca3eb5fe20b2"), QByteArray("9852564f4728e0cffe34ca3eb5fe20b2"));
    replaceList2 << qMakePair(codePrefix + encryptionZero, codePrefixRnd + codeKey);
    replaceList2 << qMakePair(assetsPrefix + encryptionZero, assetsPrefixRnd + assetsKey);
    ctx.replaceList << replaceList2;

    // copy template
    if (templatedir.length()>0)
    	ExportCommon::copyTemplate(QString("Templates").append("/").append(templatedir).append("/").append(ctx.templatename),&ctx);

    prepareAssetFolder(&ctx);
    ExportCommon::exportAllAssetsFiles(&ctx);
    if (ctx.deviceFamily == e_WinRT)
    {
    	WinRTExport::updateWinRTProject(QString("giderosgame.Windows.vcxproj"),&ctx);
        ctx.outputDir.cdUp();
        ctx.outputDir.cdUp();
        ctx.outputDir.cd("giderosgame.WindowsPhone");
        ctx.outputDir.cd("Assets");
        ExportCommon::exportAllAssetsFiles(&ctx);
        WinRTExport::updateWinRTProject(QString("giderosgame.WindowsPhone.vcxproj"),&ctx);
    }

    if (needGApp)
    {
    	ctx.outputDir.cdUp();
        if (ctx.deviceFamily == e_GApp)
        	outputDir.cdUp();
        GAppFormat::buildGApp(QDir::cleanPath(ctx.outputDir.absoluteFilePath(ctx.base+".GApp")),&ctx);
        if (ctx.deviceFamily == e_GApp)
        	ctx.outputDir.cd(ctx.base);
        else
        	ctx.outputDir.cd("assets");
        outputDir.removeRecursively();
        outputDir.cdUp();
    }

#ifdef Q_OS_MACX
    if(ctx.deviceFamily == e_MacOSXDesktop){
    	MacOSXExport::CodeSignMacOSX(&ctx);
    }
#endif
    return 0;
}

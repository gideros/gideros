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

        // graphics options
        if (!properties.attribute("scaleMode").isEmpty())
            properties_.scaleMode = properties.attribute("scaleMode").toInt();
        if (!properties.attribute("logicalWidth").isEmpty())
            properties_.logicalWidth = properties.attribute("logicalWidth").toInt();
        if (!properties.attribute("logicalHeight").isEmpty())
            properties_.logicalHeight = properties.attribute("logicalHeight").toInt();
        QDomElement imageScales = properties.firstChildElement("imageScales");
        for(QDomNode n = imageScales.firstChild(); !n.isNull(); n = n.nextSibling())
        {
            QDomElement scale = n.toElement();
            if(!scale.isNull())
                properties_.imageScales.push_back(std::make_pair(scale.attribute("suffix"), scale.attribute("scale").toDouble()));
        }
        if (!properties.attribute("orientation").isEmpty())
            properties_.orientation = properties.attribute("orientation").toInt();
        if (!properties.attribute("fps").isEmpty())
            properties_.fps = properties.attribute("fps").toInt();

        // iOS options
        if (!properties.attribute("retinaDisplay").isEmpty())
            properties_.retinaDisplay = properties.attribute("retinaDisplay").toInt();
        if (!properties.attribute("autorotation").isEmpty())
            properties_.autorotation = properties.attribute("autorotation").toInt();
        if (!properties.attribute("version").isEmpty())
            properties_.version = properties.attribute("version");
        if (!properties.attribute("version_code").isEmpty())
            properties_.version_code = properties.attribute("version_code").toInt();

        // input options
        if (!properties.attribute("mouseToTouch").isEmpty())
            properties_.mouseToTouch = properties.attribute("mouseToTouch").toInt() != 0;
        if (!properties.attribute("touchToMouse").isEmpty())
            properties_.touchToMouse = properties.attribute("touchToMouse").toInt() != 0;
        if (!properties.attribute("mouseTouchOrder").isEmpty())
            properties_.mouseTouchOrder = properties.attribute("mouseTouchOrder").toInt();

        // export options
        if (!properties.attribute("architecture").isEmpty())
            properties_.architecture = properties.attribute("architecture").toInt();
        if (!properties.attribute("assetsOnly").isEmpty())
            properties_.assetsOnly = properties.attribute("assetsOnly").toInt() != 0;
        if (!properties.attribute("iosDevice").isEmpty())
            properties_.iosDevice = properties.attribute("iosDevice").toInt();
        if (!properties.attribute("packageName").isEmpty())
            properties_.packageName = properties.attribute("packageName");
        if (!properties.attribute("encryptCode").isEmpty())
            properties_.encryptCode = properties.attribute("encryptCode").toInt() != 0;
        if (!properties.attribute("encryptAssets").isEmpty())
            properties_.encryptAssets = properties.attribute("encryptAssets").toInt() != 0;
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

enum DeviceFamily
{
  e_None,
  e_iOS,
  e_Android,
  e_WindowsDesktop,
  e_MacOSXDesktop,
  e_WinRT,
  e_GApp,
  e_Win32,
  e_Html5
};

static bool bitwiseMatchReplace(unsigned char *b,int bo,const unsigned char *m,int ms,const unsigned char *r)
{
	 if (!bo) //Simple case, no bit offset
	 {
		 if (memcmp(b,m,ms))
				 return false;
		 memcpy(b,r,ms);
		 return false;
	 }
	 for (int k=0;k<ms;k++)
	 {
		 unsigned char b1=(b[k]|(b[k+1]<<8))>>bo;
		 if (b1!=m[k]) return false;
	 }
	 for (int k=0;k<ms;k++)
	 {
		 b[k]&=(0xFF>>(8-bo));
		 b[k]|=r[k]<<bo;
		 b[k+1]&=(0xFF<<bo);
		 b[k+1]|=(r[k]>>(8-bo));
	 }
	 return true;
}

static void bitwiseReplace(char *b,int bs,const char *m,int ms,const char *r,int rs)
{
 int bmo=(bs-ms)*8;
 unsigned char *ub=(unsigned char *)b;
 const unsigned char *um=(const unsigned char *)m;
 const unsigned char *ur=(const unsigned char *)r;
 for (int k=0;k<bmo;k++)
 {
	 if (bitwiseMatchReplace(ub+(k>>3),k&7,um,ms,ur))
		 k+=(ms*8)-1;
 }
}

static void fileCopy(	const QString& srcName,
                        const QString& destName,
                        const QList<QStringList>& wildcards,
                        const QList<QList<QPair<QByteArray, QByteArray> > >& replaceList)
{
    QString srcName2 = QFileInfo(srcName).fileName();

    int match = -1;
    for (int j = 0; j < wildcards.size(); ++j)
    {
        bool submatch = false;
        for (int i = 0; i < wildcards[j].size(); ++i)
        {
            QRegExp rx(wildcards[j][i]);
            rx.setPatternSyntax(QRegExp::Wildcard);
            if (rx.exactMatch(srcName2))
            {
                submatch = true;
                break;
            }
        }
        if (submatch)
        {
            match = j;
            break;
        }
    }

    if (match != -1)
    {
        QFile in(srcName);
        if (!in.open(QFile::ReadOnly))
            return;
        QByteArray data = in.readAll();
        in.close();;

        for (int i = 0; i < replaceList[match].size(); ++i)
        	if (replaceList[match][i].first.size()==replaceList[match][i].second.size()) //Perform bitwise replacement if sizes are equal
        		bitwiseReplace(data.data(),data.size(),
        			replaceList[match][i].first.constData(),replaceList[match][i].first.size(),
        			replaceList[match][i].second.constData(),replaceList[match][i].second.size());
        	else
        		data.replace(replaceList[match][i].first, replaceList[match][i].second);

        QFile out(destName);
        if (!out.open(QFile::WriteOnly))
            return;
        out.write(data);
        out.close();
    }
    else
    {
        QFile::remove(destName);
        QFile::copy(srcName, destName);
    }
}

static bool shouldCopy(const QString &fileName, const QStringList &include, const QStringList &exclude)
{
    QString fileName2 = QFileInfo(fileName).fileName();

    bool result = false;

    for (int i = 0; i < include.size(); ++i)
    {
        QRegExp rx(include[i]);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(fileName2))
        {
            result = true;
            break;
        }
    }

    for (int i = 0; i < exclude.size(); ++i)
    {
        QRegExp rx(exclude[i]);
        rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(fileName2))
        {
            result = false;
            break;
        }
    }

    return result;
}

static void copyFolder(	const QDir& sourceDir,
                        const QDir& destDir,
                        const QList<QPair<QString, QString> >& renameList,
                        const QList<QStringList>& wildcards,
                        const QList<QList<QPair<QByteArray, QByteArray> > >& replaceList,
                        const QStringList &include,
                        const QStringList &exclude)
{
    if(!sourceDir.exists())
        return;

    QStringList files;

    files = sourceDir.entryList(QDir::Files | QDir::Hidden);
    for(int i = 0; i < files.count(); i++)
    {
        QString srcName = sourceDir.absoluteFilePath(files[i]);
        QString destFile = files[i];
        for (int i = 0; i < renameList.size(); ++i)
            destFile.replace(renameList[i].first, renameList[i].second);
        QString destName = destDir.absoluteFilePath(destFile);
        if (shouldCopy(srcName, include, exclude))
            fileCopy(srcName, destName, wildcards, replaceList);
    }

    files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i < files.count(); i++)
    {
        QDir sourceDir2 = sourceDir;
        bool b1 = sourceDir2.cd(files[i]);

        QDir destDir2 = destDir;
        QString destFile = files[i];
        for (int i = 0; i < renameList.size(); ++i)
            destFile.replace(renameList[i].first, renameList[i].second);
        destDir2.mkdir(destFile);
        bool b2 = destDir2.cd(destFile);

        if (b1 && b2)
            copyFolder(sourceDir2,
                       destDir2,
                       renameList,
                       wildcards,
                       replaceList,
                       include,
                       exclude);
    }
}

static void processOutput(QString command){
    QProcess process;
    process.execute(command);
    process.waitForFinished();
    bool commandOut = false;
    QString output = process.readAllStandardError();
    if(output.length() > 0){
        commandOut = true;
        fprintf(stderr, command.toStdString().c_str());
        fprintf(stderr, "\n");
        fprintf(stderr, output.toStdString().c_str());
        fprintf(stderr, "\n");
    }
    QString error = process.readAllStandardError();
    if(error.length() > 0){
        if(!commandOut){
            fprintf(stderr, command.toStdString().c_str());
            fprintf(stderr, "\n");
        }
        fprintf(stderr, error.toStdString().c_str());
        fprintf(stderr, "\n");
    }
}



#define BYTE_SWAP4(x) \
    (((x & 0xFF000000) >> 24) | \
     ((x & 0x00FF0000) >> 8)  | \
     ((x & 0x0000FF00) << 8)  | \
     ((x & 0x000000FF) << 24))

#define BYTE_SWAP2(x) \
    (((x & 0xFF00) >> 8) | \
     ((x & 0x00FF) << 8))

quint16 _htons(quint16 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP2(x);
    }
}

quint16 _ntohs(quint16 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP2(x);
    }
}

quint32 _htonl(quint32 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP4(x);
    }
}

quint32 _ntohl(quint32 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP4(x);
    }
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

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("GiderosMobile");
    QCoreApplication::setOrganizationDomain("giderosmobile.com");
    QCoreApplication::setApplicationName("GiderosStudio");

    QSettings::setDefaultFormat(QSettings::IniFormat);

    QCoreApplication a(argc, argv);

    QStringList arguments = a.arguments();

    QHash<QString, QString> args;

    DeviceFamily deviceFamily = e_None;
    QString projectFileName;
    QString output;
    bool encryptCode = false;
    bool encryptAssets = false;
    bool assetsOnly = false;

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
            if (platform.toLower() == "ios")
            {
                deviceFamily = e_iOS;
            }
            else if (platform.toLower() == "android")
            {
                deviceFamily = e_Android;
            }
            else if (platform.toLower() == "windows")
            {
                deviceFamily = e_WindowsDesktop;
            }
            else if (platform.toLower() == "macosx")
            {
                deviceFamily = e_MacOSXDesktop;
            }
            else if (platform.toLower() == "winrt")
            {
                deviceFamily = e_WinRT;
            }
            else if (platform.toLower() == "win32")
            {
                deviceFamily = e_Win32;
            }
            else if (platform.toLower() == "gapp")
            {
                deviceFamily = e_GApp;
            }
            else if (platform.toLower() == "html5")
            {
                deviceFamily = e_Html5;
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
            args.insert(arguments[i].remove(0,1), arguments[i + 1]);
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

    if (deviceFamily == e_None)
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
    QString templatename;
    QString templatenamews;
    bool underscore=false;
    bool needGApp=false;

    switch (deviceFamily)
    {
    case e_iOS:
      templatedir = "Xcode4";
      templatename = "iOS Template";
      templatenamews = "iOS_Template";
      underscore = true;
      break;

    case e_Android:
      templatedir = "Eclipse";
      if(args.contains("template") && args["template"] == "androidstudio")
          templatedir = "AndroidStudio";
      templatename = "Android Template";
      templatenamews = "AndroidTemplate";
      underscore = false;
      break;

    case e_WinRT:
      templatedir = "VisualStudio";
      templatename = "WinRT Template";
      templatenamews = "WinRTTemplate";
      underscore = true;
      break;

    case e_Win32:
      templatedir = "win32";
      templatename = "WindowsDesktopTemplate";
      templatenamews = "WindowsDesktopTemplate";
      underscore = true;
      break;

    case e_WindowsDesktop:
        templatedir = "Qt";
        templatename = "WindowsDesktopTemplate";
        templatenamews = "WindowsDesktopTemplate";
        underscore = false;
        break;

    case e_MacOSXDesktop:
        templatedir = "Qt";
        templatename = "MacOSXDesktopTemplate";
        templatenamews = "MacOSXDesktopTemplate";
        underscore = false;
        break;
    case e_GApp:
        underscore = false;
        needGApp = true;
        break;
    case e_Html5:
    	templatedir = "Html5";
        templatename = "Html5";
        templatenamews = "Html5";
        underscore = false;
        needGApp = true;
        break;
    }

    const QString &projectFileName_ = projectFileName;
    QString base = QFileInfo(projectFileName_).baseName();

    QString basews;
    if (underscore)
    {
        basews = base;
        for (int i = 0; i < basews.size(); ++i)
        {
            char c = basews[i].toLatin1();

            bool number = ('0' <= c) && (c <= '9');
            bool upper = ('A' <= c) && (c <= 'Z');
            bool lower = ('a' <= c) && (c <= 'z');

            if ((!number && !upper && !lower) || (number && i == 0))
                basews[i] = QChar('_');
        }
    }
    else
    {
        // 1234 Hebe Gube 456 --> HebeGube456
        bool letter = false;
        for (int i = 0; i < base.size(); ++i)
        {
            char c = base[i].toLatin1();

            bool number = ('0' <= c) && (c <= '9');
            bool upper = ('A' <= c) && (c <= 'Z');
            bool lower = ('a' <= c) && (c <= 'z');

            if (upper || lower)
                letter = true;

            if ((number || upper || lower) && letter)
                basews += base[i];
        }
    }

    outputDir.mkdir(base);
    outputDir.cd(base);

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

    if ((deviceFamily==e_Html5)&&(!args["hostname"].isEmpty()))
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

    if (deviceFamily==e_Html5)
    {
    	encryptAssets=true;
    	encryptCode=true;
    	if (!(args["hostname"].isEmpty()))
    	{
    		QByteArray mkey=args["hostname"].toUtf8();
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

    ProjectProperties properties;
    std::vector<std::pair<QString, QString> > fileQueue;
    std::vector<QString> folderList;
    DependencyGraph dependencyGraph;
    if (readProjectFile(projectFileName, properties, fileQueue, folderList, dependencyGraph) == false)
    {
        // error is displayed at readProjectFile function
        return 1;
    }

    // copy template
    if (templatedir.length()>0)
    {
        QDir dir = QDir::currentPath();
        dir.cd("Templates");
        dir.cd(templatedir);
        dir.cd(templatename);

        QList<QPair<QString, QString> > renameList;
        renameList << qMakePair(templatename, base);
        renameList << qMakePair(templatenamews, basews);

        QList<QStringList> wildcards;
        QList<QList<QPair<QByteArray, QByteArray> > > replaceList;

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
        wildcards << wildcards1;

        QList<QPair<QByteArray, QByteArray> > replaceList1;
        replaceList1 << qMakePair(templatename.toUtf8(), base.toUtf8());
        replaceList1 << qMakePair(templatenamews.toLatin1(), basews.toLatin1());
        if (deviceFamily == e_Android){
            replaceList1 << qMakePair(QString("com.giderosmobile.androidtemplate").toUtf8(), args["package"].toUtf8());
            replaceList1 << qMakePair(QString("android:versionCode=\"1\"").toUtf8(), ("android:versionCode=\""+QString::number(properties.version_code)+"\"").toUtf8());
            replaceList1 << qMakePair(QString("android:versionName=\"1.0\"").toUtf8(), ("android:versionName=\""+properties.version+"\"").toUtf8());
            QString orientation = "android:screenOrientation=\"portrait\"";
            switch(properties.orientation){
                case 0:
                    if(properties.autorotation > 0)
                        orientation = "android:screenOrientation=\"sensorPortrait\"";
                    else
                        orientation = "android:screenOrientation=\"portrait\"";
                    break;
                case 1:
                    if(properties.autorotation > 0)
                        orientation = "android:screenOrientation=\"sensorLandscape\"";
                    else
                        orientation = "android:screenOrientation=\"landscape\"";
                    break;
                case 2:
                    if(properties.autorotation > 0)
                        orientation = "android:screenOrientation=\"sensorPortrait\"";
                    else
                        orientation = "android:screenOrientation=\"reversePortrait\"";
                    break;
                case 3:
                    if(properties.autorotation > 0)
                        orientation = "android:screenOrientation=\"sensorLandscape\"";
                    else
                        orientation = "android:screenOrientation=\"reverseLandscape\"";
                    break;
            }

            replaceList1 << qMakePair(QString("android:screenOrientation=\"portrait\"").toUtf8(), orientation.toUtf8());
        }
        else if(deviceFamily == e_MacOSXDesktop){
            QString category = "public.app-category.games";
            if(args.contains("category"))
                category = args["category"];
            if(args.contains("bundle"))
                replaceList1 << qMakePair(QString("com.yourcompany."+base).toUtf8(), args["bundle"].toUtf8());
            replaceList1 << qMakePair(QString("<key>NOTE</key>").toUtf8(), ("<key>LSApplicationCategoryType</key>\n	<string>"+category.toUtf8()+"</string>\n	<key>CFBundleShortVersionString</key>\n	<string>"+properties.version+"</string>\n	<key>CFBundleVersion</key>\n	<string>"+properties.version+"</string>\n	<key>CFBundleName</key>\n	<string>"+base.toUtf8()+"</string>\n	<key>NOTE</key>").toUtf8());
        }
        else if(deviceFamily == e_iOS){
            if(args.contains("bundle"))
                replaceList1 << qMakePair(QString("com.yourcompany.${PRODUCT_NAME:rfc1034identifier}").toUtf8(), args["bundle"].toUtf8());
            replaceList1 << qMakePair(QString("<string>1.0</string>").toUtf8(), ("<string>"+properties.version+"</string>").toUtf8());
        }
        else if(deviceFamily == e_WinRT){
            replaceList1 << qMakePair(QString("Gideros Player").toUtf8(), base.toUtf8());
            replaceList1 << qMakePair(QString("giderosgame").toUtf8(), basews.toUtf8());
            replaceList1 << qMakePair(QString("com.giderosmobile.windowsphone").toUtf8(), args["package"].toUtf8());
            replaceList1 << qMakePair(QString("com.giderosmobile.windows").toUtf8(), args["package"].toUtf8());
            replaceList1 << qMakePair(QString("Gideros Mobile").toUtf8(), args["organization"].toUtf8());
        }
        else if(deviceFamily == e_Html5){
            replaceList1 << qMakePair(QString("<title>Gideros</title>").toUtf8(), ("<title>"+base+"</title>").toUtf8());
            replaceList1 << qMakePair(QString("gideros.GApp").toUtf8(), (base+".GApp").toUtf8());
        }
        replaceList << replaceList1;

            QStringList wildcards2;
            wildcards2 << "libgideros.so" << "libgideros.a" << "gid.dll"
            		   << "libgid.1.dylib" << "gideros.WindowsPhone.lib"
            		   << "gideros.Windows.lib" << "gideros.html.mem";
            wildcards << wildcards2;

            QList<QPair<QByteArray, QByteArray> > replaceList2;
            replaceList2 << qMakePair(QByteArray("9852564f4728e0c11e34ca3eb5fe20b2"), QByteArray("9852564f4728e0cffe34ca3eb5fe20b2"));
            replaceList2 << qMakePair(codePrefix + encryptionZero, codePrefixRnd + codeKey);
            replaceList2 << qMakePair(assetsPrefix + encryptionZero, assetsPrefixRnd + assetsKey);
            replaceList << replaceList2;

        if (assetsOnly)
            copyFolder(dir, outputDir, renameList, wildcards, replaceList, QStringList() << "libgideros.so" << "libgideros.a" << "gideros.jar" << "gideros.dll" << "libgideros.dylib" << "libgideros.1.dylib" << "gideros.WindowsPhone.lib" << "gideros.Windows.lib" << "WindowsDesktopTemplate.exe" << "MacOSXDesktopTemplate", QStringList());
        else
            copyFolder(dir, outputDir, renameList, wildcards, replaceList, QStringList() << "*", QStringList());
    }

    if (deviceFamily == e_iOS)
    {
        outputDir.mkdir(base);
        outputDir.cd(base);
    }
    else if (deviceFamily == e_Android)
    {
        if(args["template"] == "androidstudio"){
            outputDir.cd("app");
            outputDir.cd("src");
            outputDir.cd("main");
            outputDir.mkdir("assets");
            outputDir.cd("assets");
        }
        else{
            outputDir.mkdir("assets");
            outputDir.cd("assets");
        }
    }
    else if(deviceFamily == e_MacOSXDesktop)
    {
        outputDir.cd(base + ".app");
        outputDir.cd("Contents");
    }
    else if (deviceFamily == e_WinRT)
    {
      outputDir.cd("giderosgame");
      outputDir.cd("giderosgame.Windows");
      outputDir.cd("Assets");
    }

    if (deviceFamily != e_WinRT){
      outputDir.mkdir("assets");
      outputDir.cd("assets");
    }

        if(deviceFamily == e_MacOSXDesktop || deviceFamily == e_WindowsDesktop){
            QString org;
            QString domain;
            if(deviceFamily == e_MacOSXDesktop){
                org = args["organization"];
                domain = args["domain"];
            }
            else if(deviceFamily == e_WindowsDesktop){
                org = args["organization"];
                domain = args["domain"];
            }
            QString filename = "data.bin";
            QFile file(QDir::cleanPath(outputDir.absoluteFilePath(filename)));
            if (file.open(QIODevice::WriteOnly))
            {
                ByteBuffer buffer;

                buffer << org.toStdString().c_str();
                buffer << domain.toStdString().c_str();
                buffer << base.toStdString().c_str();

                file.write(buffer.data(), buffer.size());
            }
            outputDir.mkdir("resource");
            outputDir.cd("resource");
        }


        for (std::size_t i = 0; i < folderList.size(); ++i){
            outputDir.mkdir(folderList[i]);
            if (deviceFamily == e_WinRT){
                  outputDir.cdUp();
                  outputDir.cdUp();
                  outputDir.cd("giderosgame.WindowsPhone");
                  outputDir.cd("Assets");

                  outputDir.mkdir(folderList[i]);

                  outputDir.cdUp();
                  outputDir.cdUp();
                  outputDir.cd("giderosgame.Windows");
                  outputDir.cd("Assets");
            }
        }

        std::vector<std::pair<QString, bool> > topologicalSort = dependencyGraph.topologicalSort();
        for (std::size_t i = 0; i < topologicalSort.size(); ++i)
        {
            int index = -1;
            for (std::size_t j = 0; j < fileQueue.size(); ++j)
            {
                if (fileQueue[j].second == topologicalSort[i].first)
                {
                    index = j;
                    break;
                }
            }

            if (index != -1)
            {
                std::pair<QString, QString> item = fileQueue[index];
                fileQueue.erase(fileQueue.begin() + index);
                fileQueue.push_back(item);
            }
        }

        int npass,ipass;

        if (deviceFamily == e_WinRT)
          npass=2;
        else
          npass=1;

        QStringList luafiles;
        QStringList luafiles_abs;
        QStringList allfiles;
        QStringList allfiles_abs;
        QStringList allluafiles;
        QStringList allluafiles_abs;

    for (ipass=1;ipass<=npass;ipass++)
    {

      luafiles.clear();
      luafiles_abs.clear();
      allfiles.clear();
      allfiles_abs.clear();
      allluafiles.clear();
      allluafiles_abs.clear();

      if (ipass==2){
        outputDir.cdUp();
        outputDir.cdUp();
        outputDir.cd("giderosgame.WindowsPhone");
        outputDir.cd("Assets");
      }

        QSet<QString> jetset;
        jetset << "mp3" << "png" << "jpg" << "jpeg" << "wav";

        QDir path(QFileInfo(projectFileName_).path());

        for (std::size_t i = 0; i < fileQueue.size(); ++i)
        {
            const QString& s1 = fileQueue[i].first;
            const QString& s2 = fileQueue[i].second;

            QString src = QDir::cleanPath(path.absoluteFilePath(s2));
            QString dst = QDir::cleanPath(outputDir.absoluteFilePath(s1));

            if (deviceFamily == e_Android)
            {
                QString suffix = QFileInfo(dst).suffix().toLower();
                if (!jetset.contains(suffix))
                    dst += ".jet";
            }

            allfiles.push_back(s1);
            allfiles_abs.push_back(dst);

            if (QFileInfo(src).suffix().toLower() == "lua")
            {
                allluafiles.push_back(s1);
                allluafiles_abs.push_back(dst);

                if (std::find(topologicalSort.begin(), topologicalSort.end(), std::make_pair(s2, true)) == topologicalSort.end())
                {
                    luafiles.push_back(s1);
                    luafiles_abs.push_back(dst);
                }
            }

            QFile::remove(dst);
            QFile::copy(src, dst);
        }

    #if 0
        // compile lua files
        if (false)
        {
            compileThread_ = new CompileThread(luafiles_abs, false, "", QString(), this);
            compileThread_->start();
            compileThread_->wait();
            delete compileThread_;
        }
    #endif

        // compile lua files (with luac)
        // disable compile with luac for iOS because 64 bit version
        // http://giderosmobile.com/forum/discussion/5380/ios-8-64bit-only-form-feb-2015
        if (deviceFamily != e_iOS && deviceFamily != e_MacOSXDesktop)
        {
            for (int i = 0; i < allluafiles_abs.size(); ++i)
            {
                QString file = "\"" + allluafiles_abs[i] + "\"";
                QProcess::execute("Tools/luac -o " + file + " " + file);
            }
        }

        // encrypt lua, png, jpg, jpeg and wav files
        if (true)
        {
            for (int i = 0; i < allfiles_abs.size(); ++i)
            {
                QString ext = QFileInfo(allfiles[i]).suffix().toLower();
                if (ext != "lua" && ext != "png" && ext != "jpeg" && ext != "jpg" && ext != "wav")
                    continue;

                QByteArray encryptionKey = (ext == "lua") ? codeKey : assetsKey;

                QString filename = allfiles_abs[i];

                QFile fis(filename);
                if (!fis.open(QIODevice::ReadOnly))
                    continue;
                QByteArray data = fis.readAll();
                fis.close();

                int ks=encryptionKey.size();
                for (int j = 32; j < data.size(); ++j)
                    data[j] = data[j] ^ encryptionKey[((j*13)+((j/ks)*31)) % ks];

                QFile fos(filename);
                if (!fos.open(QIODevice::WriteOnly))
                    continue;
                fos.write(data);
                fos.close();
            }
        }

        // compress lua files
        if (false)
        {
            for (int i = 0; i < luafiles_abs.size(); ++i)
            {
                QString file = "\"" + luafiles_abs[i] + "\"";
                QProcess::execute("Tools/lua Tools/LuaSrcDiet.lua --quiet " + file + " -o " + file);
            }
        }

    // ----------------------------------------------------------------------
    // For WinRT, write all asset filenames into giderosgame.Windows.vcxproj
    // ----------------------------------------------------------------------

        if (deviceFamily == e_WinRT){

          outputDir.cdUp();

          QByteArray replacement;
          for (int i = 0; i < allfiles.size(); i++){
            QString assetfile=allfiles[i];
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

          QString projfile;

          if (ipass==1)
            projfile="giderosgame.Windows.vcxproj";
          else
            projfile="giderosgame.WindowsPhone.vcxproj";

          QByteArray data;
          QFile in(outputDir.absoluteFilePath(projfile));
          in.open(QFile::ReadOnly);

          data = in.readAll();
          in.close();

          data.replace("INSERT_ASSETS_HERE",replacement);

          QFile out(outputDir.absoluteFilePath(projfile));
          out.open(QFile::WriteOnly);

          out.write(data);
          out.close();

          outputDir.cd("Assets");
        }

    // ----------------------------------------------------------------------


        if(deviceFamily == e_MacOSXDesktop || deviceFamily == e_WindowsDesktop)
        {
            outputDir.cd("..");
        }

        // write luafiles.txt
        {
            QString filename = "luafiles.txt";
            if (deviceFamily == e_Android)
                filename += ".jet";
            QFile file(QDir::cleanPath(outputDir.absoluteFilePath(filename)));
            if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&file);

                for (int i = 0; i < luafiles.size(); ++i)
                    out << luafiles[i] << "\n";
            }
            if (needGApp)
            {
                allfiles.push_back(filename);
                allfiles_abs.push_back(QDir::cleanPath(outputDir.absoluteFilePath(filename)));
            }
        }

        // write allfiles.txt
        if (deviceFamily == e_Android)
        {
            QFile file(QDir::cleanPath(outputDir.absoluteFilePath("allfiles.txt")));
            if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream out(&file);

                for (int i = 0; i < allfiles.size(); ++i)
                {
                    QString file = allfiles[i];
                    QString suffix = QFileInfo(file).suffix().toLower();
                    if (!jetset.contains(suffix))
                        file += "*";
                    out << file << "\n";
                }
            }
        }

        // write properties.bin
        {
            QString filename = "properties.bin";
            if (deviceFamily == e_Android)
                filename += ".jet";
            QFile file(QDir::cleanPath(outputDir.absoluteFilePath(filename)));
            if (file.open(QIODevice::WriteOnly))
            {

                ByteBuffer buffer;

                buffer << properties.scaleMode;
                buffer << properties.logicalWidth;
                buffer << properties.logicalHeight;

                buffer << (int)properties.imageScales.size();
                for (size_t i = 0; i < properties.imageScales.size(); ++i)
                {
                    buffer << properties.imageScales[i].first.toUtf8().constData();
                    buffer << (float)properties.imageScales[i].second;
                }

                buffer << properties.orientation;
                buffer << properties.fps;

                buffer << properties.retinaDisplay;
                buffer << properties.autorotation;

                buffer << (properties.mouseToTouch ? 1 : 0);
                buffer << (properties.touchToMouse ? 1 : 0);
                buffer << properties.mouseTouchOrder;

                buffer << properties.windowWidth;
                buffer << properties.windowHeight;

                file.write(buffer.data(), buffer.size());
            }
            if (needGApp)
            {
                allfiles.push_back(filename);
                allfiles_abs.push_back(QDir::cleanPath(outputDir.absoluteFilePath(filename)));
            }
        }

    }  // end of ipass loop

    if (needGApp)
    {
        outputDir.cdUp();
        if (deviceFamily == e_GApp)
        	outputDir.cdUp();

        QFile file(QDir::cleanPath(outputDir.absoluteFilePath(base+".GApp")));
        if (file.open(QIODevice::WriteOnly))
        {

            ByteBuffer buffer;
            quint32 offset=0;
            quint32 cbuf;
            char cpbuf[4096];
            for (int k=0;k<allfiles.size();k++)
            {
                buffer.append(allfiles[k].toStdString());
                cbuf=_htonl(offset);
                buffer.append((unsigned char *) &cbuf,4);
                QFile src(allfiles_abs[k]);
                src.open(QIODevice::ReadOnly);
                int size=0;
                while (true)
                {
                    int rd=src.read(cpbuf,sizeof(cpbuf));
                    if (rd<=0) break;
                    size+=rd;
                    file.write(cpbuf,rd);
                    if (rd<sizeof(cpbuf))
                        break;
                }
                src.close();
                cbuf=_htonl(size);
                buffer.append((unsigned char *) &cbuf,4);
                offset+=size;
            }
            buffer.append(""); //End of file list marker, i.e. empty filename
            if (offset&7)
                offset+=file.write(cpbuf,8-(offset&7)); // Align structure to 8 byte boundary
            cbuf=_htonl(offset); //File list offset from beginning of package
            buffer.append((unsigned char *) &cbuf,4);
            cbuf=_htonl(0); //Version
            buffer.append((unsigned char *) &cbuf,4);
            buffer.append("GiDeRoS"); //Package marker
            file.write(buffer.data(), buffer.size());
        }
        file.close();
        if (deviceFamily == e_GApp)
        	outputDir.cd(base);
        else
        	outputDir.cd("assets");
        outputDir.removeRecursively();
        outputDir.cdUp();
    }

#ifdef Q_OS_MACX
    if(deviceFamily == e_MacOSXDesktop){
        outputDir.cdUp();
        outputDir.cd("Frameworks");
        QString script = "";
        QProcess postProcess;
        QString cmd;
        QStringList frameworks = outputDir.entryList(QStringList() << "*.framework");
        for(int i = 0; i < frameworks.size(); ++i){
            QString filename = outputDir.absoluteFilePath(frameworks[i]);
            cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+args["organization"]+"\" \""+filename+"/Versions/Current\"";
            script += cmd+"\n";
            processOutput(cmd);
        }
        QStringList dylibs = outputDir.entryList(QStringList() << "*.dylib");
        for(int i = 0; i < dylibs.size(); ++i){
            QString filename = outputDir.absoluteFilePath(dylibs[i]);
            cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+args["organization"]+"\" \""+filename+"\"";
            script += cmd+"\n";
            processOutput(cmd);
        }

        outputDir.cdUp();
        outputDir.cd("PlugIns");
        dylibs = outputDir.entryList(QStringList() << "*.dylib");
        for(int i = 0; i < dylibs.size(); ++i){
            QString filename = outputDir.absoluteFilePath(dylibs[i]);
            cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+args["organization"]+"\" \""+filename+"\"";
            script += cmd+"\n";
            processOutput(cmd);
        }

        outputDir.cd("bearer");
        dylibs = outputDir.entryList(QStringList() << "*.dylib");
        for(int i = 0; i < dylibs.size(); ++i){
            QString filename = outputDir.absoluteFilePath(dylibs[i]);
            cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+args["organization"]+"\" \""+filename+"\"";
            script += cmd+"\n";
            processOutput(cmd);
        }

        outputDir.cdUp();
        outputDir.cd("imageformats");
        dylibs = outputDir.entryList(QStringList() << "*.dylib");
        for(int i = 0; i < dylibs.size(); ++i){
            QString filename = outputDir.absoluteFilePath(dylibs[i]);
            cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+args["organization"]+"\" \""+filename+"\"";
            script += cmd+"\n";
            processOutput(cmd);
        }

        outputDir.cdUp();
        outputDir.cd("platforms");
        dylibs = outputDir.entryList(QStringList() << "*.dylib");
        for(int i = 0; i < dylibs.size(); ++i){
            QString filename = outputDir.absoluteFilePath(dylibs[i]);
            cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+args["organization"]+"\" \""+filename+"\"";
            script += cmd+"\n";
            processOutput(cmd);
        }

        outputDir.cdUp();
        outputDir.cd("printsupport");
        dylibs = outputDir.entryList(QStringList() << "*.dylib");
        for(int i = 0; i < dylibs.size(); ++i){
            QString filename = outputDir.absoluteFilePath(dylibs[i]);
            cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+args["organization"]+"\" \""+filename+"\"";
            script += cmd+"\n";
            processOutput(cmd);
        }

        outputDir.cdUp();
        outputDir.cd("audio");
        dylibs = outputDir.entryList(QStringList() << "*.dylib");
        for(int i = 0; i < dylibs.size(); ++i){
            QString filename = outputDir.absoluteFilePath(dylibs[i]);
            cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+args["organization"]+"\" \""+filename+"\"";
            script += cmd+"\n";
            processOutput(cmd);
        }

        outputDir.cdUp();
        outputDir.cd("mediaservice");
        dylibs = outputDir.entryList(QStringList() << "*.dylib");
        for(int i = 0; i < dylibs.size(); ++i){
            QString filename = outputDir.absoluteFilePath(dylibs[i]);
            cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+args["organization"]+"\" \""+filename+"\"";
            script += cmd+"\n";
            processOutput(cmd);
        }

        outputDir.cdUp();
        outputDir.cdUp();
        outputDir.cdUp();
        outputDir.cdUp();
        cmd = "codesign -f -s \"3rd Party Mac Developer Application: "+args["organization"]+"\" --entitlements \"/"+outputDir.absolutePath()+"/Entitlements.plist\" \""+outputDir.absoluteFilePath(base + ".app")+"\"";
        script += cmd+"\n";
        processOutput(cmd);

        cmd = "productbuild --component \""+outputDir.absoluteFilePath(base + ".app")+"\" /Applications --sign \"3rd Party Mac Developer Installer: "+args["organization"]+"\" \""+outputDir.absoluteFilePath(base + ".pkg")+"\"";
        script += cmd+"\n";
        processOutput(cmd);

        QFile file(outputDir.absoluteFilePath("package.sh"));
        file.open(QIODevice::WriteOnly);
        file.write(script.toStdString().c_str(), qstrlen(script.toStdString().c_str()));
        file.close();
    }
#endif
    return 0;
}

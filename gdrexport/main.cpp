#include <licensemanager.h>
#include <QDir>
#include <QSettings>
#include <time.h>
#include <QDomDocument>
#include <projectproperties.h>
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
    e_None = -1,
    e_iOS,
    e_Android,
};

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
            data.replace(replaceList[match][i].first, replaceList[match][i].second);

        QFile out(destName);
        if (!out.open(QFile::WriteOnly))
            return;
        out.write(data);
        out.close();
    }
    else
    {
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


void usage()
{
    fprintf(stderr, "Usage: gdrexport -platform <platform_name> -package <package_name> -encrypt -encrypt-code -encrypt-assets -assets-only <project_file> <output_dir>\n");
}

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("GiderosMobile");
    QCoreApplication::setOrganizationDomain("giderosmobile.com");
    QCoreApplication::setApplicationName("GiderosStudio");

    QSettings::setDefaultFormat(QSettings::IniFormat);

    QCoreApplication a(argc, argv);

    QStringList arguments = a.arguments();

    DeviceFamily deviceFamily = e_None;
    QString packageName;
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
        else if (arguments[i] == "-package")
        {
            if (i + 1 >= arguments.size())
            {
                fprintf(stderr, "Missing argument: package_name\n\n");
                usage();
                return 1;
            }
            packageName = arguments[i + 1];
            i += 2;
        }
        else if (arguments[i].startsWith("-"))
        {
            fprintf(stderr, "Unknown argument: %s\n\n", qPrintable(arguments[i]));
            usage();
            return 1;
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

    if (deviceFamily == e_Android && packageName.isEmpty())
    {
        fprintf(stderr, "Missing argument: package_name\n\n");
        usage();
        return 1;
    }

    LicenseManager licenseManager;
    int licenseType = licenseManager.getLicenseType();

    if (licenseType == -1)
    {
        fprintf(stderr, "You do not have a license installed. Please run Gideros License Manager.\n");
        return 1;
    }

    if (g_checkHash() == false)
    {
        fprintf(stderr, "Inconsistent license file. Please run Gideros License Manager and update your license.\n");
        return 1;
    }

    projectFileName = QDir::current().absoluteFilePath(projectFileName);

    QDir dir = QCoreApplication::applicationDirPath();
    dir.cdUp();
    QDir::setCurrent(dir.absolutePath());

    bool licensed = (licenseType == 2 || licenseType == 3);

    if (licensed == false)
    {
        encryptCode = false;
        encryptAssets = false;
    }

    QString templatedir;
    QString templatename;
    QString templatenamews;
    bool underscore;

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
            templatename = "Android Template";
            templatenamews = "AndroidTemplate";
            underscore = false;
            break;
    }

    QDir outputDir(output);

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
    QByteArray encryptionZero(16, '\0');
    QByteArray codeKey(16, '\0');
    QByteArray assetsKey(16, '\0');
    QByteArray randomKey(16, '\0');
    qsrand(time(NULL));
    for (int i = 0; i < 16; ++i)
        randomKey[i] = qrand() % 256;
    if (encryptCode)
    {
        QSettings settings;
        if (settings.contains("codeKey"))
        {
            codeKey = settings.value("codeKey").toByteArray();
        }
        else
        {
            codeKey = randomKey;
            settings.setValue("codeKey", codeKey);
            settings.sync();
        }
    }
    if (encryptAssets)
    {
        QSettings settings;
        if (settings.contains("assetsKey"))
        {
            assetsKey = settings.value("assetsKey").toByteArray();
        }
        else
        {
            assetsKey = randomKey;
            settings.setValue("assetsKey", assetsKey);
            settings.sync();
        }
    }

    // copy template
    if (true)
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
            "*.project";
        wildcards << wildcards1;

        QList<QPair<QByteArray, QByteArray> > replaceList1;
        replaceList1 << qMakePair(templatename.toUtf8(), base.toUtf8());
        replaceList1 << qMakePair(templatenamews.toLatin1(), basews.toLatin1());
        if (deviceFamily == e_Android)
            replaceList1 << qMakePair(QString("com.giderosmobile.androidtemplate").toUtf8(), packageName.toUtf8());
        replaceList << replaceList1;

        if (licensed)
        {
            QStringList wildcards2;
            wildcards2 << "libgideros.so" << "libgideros.a";
            wildcards << wildcards2;

            QList<QPair<QByteArray, QByteArray> > replaceList2;
            replaceList2 << qMakePair(QByteArray("9852564f4728e0c11e34ca3eb5fe20b2"), QByteArray("9852564f4728e0cffe34ca3eb5fe20b2"));
            replaceList2 << qMakePair(codePrefix + encryptionZero, codePrefix + codeKey);
            replaceList2 << qMakePair(assetsPrefix + encryptionZero, assetsPrefix + assetsKey);
            replaceList << replaceList2;
        };

        if (assetsOnly)
            copyFolder(dir, outputDir, renameList, wildcards, replaceList, QStringList() << "libgideros.so" << "libgideros.a" << "gideros.jar", QStringList());
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
        outputDir.mkdir("assets");
        outputDir.cd("assets");
    }
    outputDir.mkdir("assets");
    outputDir.cd("assets");


    ProjectProperties properties;
    std::vector<std::pair<QString, QString> > fileQueue;
    std::vector<QString> folderList;
    DependencyGraph dependencyGraph;
    if (readProjectFile(projectFileName, properties, fileQueue, folderList, dependencyGraph) == false)
    {
		// error is displayed at readProjectFile function
        return 1;
    }

    for (std::size_t i = 0; i < folderList.size(); ++i)
        outputDir.mkdir(folderList[i]);

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

    QStringList luafiles;
    QStringList luafiles_abs;
    QStringList allfiles;
    QStringList allfiles_abs;
    QStringList allluafiles;
    QStringList allluafiles_abs;

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

    // compile lua files (with luac)
    if (true)
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

            for (int j = 0; j < data.size(); ++j)
                data[j] = data[j] ^ encryptionKey[j % encryptionKey.size()];

            QFile fos(filename);
            if (!fos.open(QIODevice::WriteOnly))
                continue;
            fos.write(data);
            fos.close();
        }
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

            file.write(buffer.data(), buffer.size());
        }
    }

    return 0;
}

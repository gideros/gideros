#include "application.h"
#include <QDebug>
#include <QTimer>
#include <giderosnetworkclient2.h>
#if USE_LOCAL_SOCKETS
#include <QLocalServer>
#include <QLocalSocket>
#else
#include <QTcpServer>
#include <QTcpSocket>
#endif
#include <QDataStream>
#include <QDomDocument>
#include <QFile>
#include <stack>
#include <QFileInfo>
#include <QDir>
#include <bytebuffer.h>
#include <QCoreApplication>

#include <sys/stat.h>
#include <time.h>

static time_t fileLastModifiedTime(const char* file)
{
    struct stat s;
    stat(file, &s);

    return s.st_mtime;
}

static time_t fileAge(const char* file)
{
    return time(NULL) - fileLastModifiedTime(file);
}

Application::Application(QObject *parent) :
    QObject(parent)
{
    client_ = new GiderosNetworkClient("127.0.0.1", 15000, this);
    connect(client_, SIGNAL(connected()), this, SLOT(connected()));
    connect(client_, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(client_, SIGNAL(dataReceived(const QByteArray&)), this, SLOT(dataReceived(const QByteArray&)));
    connect(client_, SIGNAL(ackReceived(unsigned int)), this, SLOT(ackReceived(unsigned int)));

#if USE_LOCAL_SOCKETS
    server_ = new QLocalServer(this);
    server_->listen("gdrdeamon");

    server2_ = new QLocalServer(this);
    server2_->listen("gdrdeamon2");
#else
    server_ = new QTcpServer(this);
    server_->listen(QHostAddress::Any, 15001);

    server2_ = new QTcpServer(this);
    server2_->listen(QHostAddress::Any, 15002);
#endif
    connect(server_, SIGNAL(newConnection()), this, SLOT(newConnection()));
    connect(server2_, SIGNAL(newConnection()), this, SLOT(ignoreConnection()));

    timer_ = new QTimer(this);
    connect(timer_, SIGNAL(timeout()), this, SLOT(timer()));
    timer_->start(1000 / 30);

    isTransferring_ = false;
    time_.start();
}

void Application::connected()
{
    fileQueue_.clear();
    isTransferring_ = false;
}

void Application::disconnected()
{
    fileQueue_.clear();
    isTransferring_ = false;
    time_.setHMS(0, 0, 0);
    time_.start();
}

void Application::dataReceived(const QByteArray& d)
{
    //	if (!updateMD5().empty())	// bunu neden koymusuz acaba?
    //		saveMD5();

        const char* data = d.constData();

        if (data[0] == 4)
        {
            std::string str = &data[1];
            //outputWidget_->append(QString::fromUtf8(str.c_str()));
            log_ << QString::fromUtf8(str.c_str());
        }
        if (data[0] == 6 && isTransferring_ == true)
        {
            //printf("file list has got\n");

            fileQueue_.clear();

            std::map<QString, QString> localFileMap;
            std::map<QString, QString> localFileMapReverse;
            {
                std::vector<std::pair<QString, QString> > fileList = fileList_;

                for (std::size_t i = 0; i < fileList.size(); ++i)
                {
                    localFileMap[fileList[i].first] = fileList[i].second;
                    localFileMapReverse[fileList[i].second] = fileList[i].first;
                }
            }

            std::map<QString, std::pair<int, QByteArray> > remoteFileMap;

            ByteBuffer buffer(d.constData(), d.size());

            char chr;
            buffer >> chr;

            while (buffer.eob() == false)
            {
                std::string file;
                buffer >> file;

                if (file[0] == 'F')
                {
                    int age;
                    buffer >> age;

                    unsigned char md5[16];
                    buffer.get(md5, 16);

                    remoteFileMap[file.c_str() + 1] = std::make_pair(age, QByteArray((char*)md5, 16));
                }
                else if (file[0] == 'D')
                {
                }
            }

            // delete unused files
            for (std::map<QString, std::pair<int, QByteArray> >::iterator iter = remoteFileMap.begin(); iter != remoteFileMap.end(); ++iter)
            {
                if (localFileMap.find(iter->first) == localFileMap.end())
                {
                    //printf("deleting: %s\n", qPrintable(iter->first));
                    client_->sendDeleteFile(qPrintable(iter->first));
                }
            }

            // upload files
            QDir path(QFileInfo(projectFileName_).path());
            for (std::map<QString, QString>::iterator iter = localFileMap.begin(); iter != localFileMap.end(); ++iter)
            {
                std::map<QString, std::pair<int, QByteArray> >::iterator riter = remoteFileMap.find(iter->first);

                QString localfile = QDir::cleanPath(path.absoluteFilePath(iter->second));

                bool send = false;
                if (riter == remoteFileMap.end())
                {
                    //printf("always upload: %s\n", qPrintable(iter->first));
                    send = true;
                }
                else
                {
                    int localage = fileAge(qPrintable(localfile));
                    int remoteage = riter->second.first;
                    const QByteArray& localmd5 = md5_[iter->second].second;
                    const QByteArray& remotemd5 = riter->second.second;

                    if (localage < remoteage || localmd5 != remotemd5)
                    {
                        //printf("upload new file: %s\n", qPrintable(iter->first));
                        send = true;
                    }
                }

                if (send == true)
                    fileQueue_.push_back(qMakePair(iter->first, localfile));
                else
                {
                    //printf("don't upload: %s\n", qPrintable(iter->first));
                }
            }

            std::vector<std::pair<QString, bool> > topologicalSort = dependencyGraph_.topologicalSort();

            QStringList luaFiles;
            for (std::size_t i = 0; i < topologicalSort.size(); ++i)
                if (topologicalSort[i].second == false)
                    luaFiles << localFileMapReverse[topologicalSort[i].first];

            if (luaFiles.empty() == false)
                fileQueue_.push_back(qMakePair(luaFiles.join("|"), QString("play")));
        }

}

void Application::ackReceived(unsigned int id)
{
}

void Application::timer()
{
    if (!client_->isConnected() && time_.elapsed() > 5000)
        QCoreApplication:exit(0);

    QDir path(QFileInfo(projectFileName_).path());

    if (client_ && client_->bytesToWrite() == 0)
    {
        if (fileQueue_.empty() == false)
        {
            const QString& s1 = fileQueue_.front().first;
            const QString& s2 = fileQueue_.front().second;

            if (s2 == "play")
            {
                QStringList luafiles = s1.split("|");
                //outputWidget_->append("Uploading finished.\n");
                client_->sendProjectProperties(properties_);
                client_->sendPlay(luafiles);
                isTransferring_ = false;
            }
            else
            {
                // create remote directories
                {
                    QStringList paths = s1.split("/");
                    QStringList dir;
                    for (int i = 0; i < paths.size() - 1; ++i)
                    {
                        dir << paths[i];
                        client_->sendCreateFolder(dir.join("/"));
                    }
                }

                QString fileName = QDir::cleanPath(path.absoluteFilePath(s2));
                if (client_->sendFile(s1, fileName) == 0)
                {
                    //outputWidget_->append(s1 + " cannot be opened.\n");
                }
                else
                {
                    //outputWidget_->append(s1 + " is uploading.\n");
                }
            }

            fileQueue_.pop_front();
        }
    }
}

void Application::ignoreConnection()
{
    QTcpSocket *socket = server2_->nextPendingConnection();
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
    socket->disconnectFromHost();
}

void Application::newConnection()
{
#if USE_LOCAL_SOCKETS
    QLocalSocket *socket = server_->nextPendingConnection();
#else
    QTcpSocket *socket = server_->nextPendingConnection();
#endif

    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

    QByteArray in;
    while (socket->waitForReadyRead())
    {
        in.append(socket->readAll());

        if (in.size() >= sizeof(qint32))
        {
            qint32 size = *(qint32*)in.data();
            if (in.size() >= size)
                break;
        }
    }
    in.remove(0, sizeof(qint32));

    QDataStream instream(in);

    QString command;
    instream >> command;

    QByteArray out;
    QDataStream outstream(&out, QIODevice::WriteOnly);

    if (command == "play")
    {
        QString fileName;
        instream >> fileName;

        play(fileName);
    }
    else if (command == "stop")
    {
        stop();
    }
    else if (command == "setip")
    {
        QString ip;
        instream >> ip;

        client_->connectToHost(ip, 15000);
    }
    else if (command == "isconnected")
    {
        outstream << QString(client_->isConnected() ? "1" : "0");
    }
    else if (command == "getlog")
    {
        for (int i = 0; i < log_.size(); ++i)
            outstream << log_[i];
        log_.clear();
    }
    else if (command == "stopdeamon")
    {
        QCoreApplication::exit(0);
    }

    socket->write(out);
    while (socket->bytesToWrite() && socket->waitForBytesWritten())
        ;

    socket->flush();

#if USE_LOCAL_SOCKETS
    socket->disconnectFromServer();
#else
    socket->disconnectFromHost();
#endif

    time_.setHMS(0, 0, 0);
    time_.start();
}

void Application::play(const QString &fileName)
{
    if (!client_->isConnected())
        return;

    if (isTransferring_)
        return;

    QDomDocument doc;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly))
        return;

    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }

    file.close();

    isTransferring_ = true;

    projectFileName_ = fileName;

    // read properties
    {
        QDomElement root = doc.documentElement();

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

        // export options
        if (!properties.attribute("architecture").isEmpty())
            properties_.architecture = properties.attribute("architecture").toInt();
        if (!properties.attribute("assetsOnly").isEmpty())
            properties_.assetsOnly = properties.attribute("assetsOnly").toInt() != 0;
        if (!properties.attribute("iosDevice").isEmpty())
            properties_.iosDevice = properties.attribute("iosDevice").toInt();
        if (!properties.attribute("packageName").isEmpty())
            properties_.packageName = properties.attribute("packageName");
    }

    // populate file list and dependency graph
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

    loadMD5();
    if (!updateMD5().empty())
        saveMD5();

    client_->sendStop();
    client_->sendProjectName(QFileInfo(projectFileName_).baseName());
    client_->sendGetFileList();
}


void Application::loadMD5()
{
    md5_.clear();

    QDir dir = QFileInfo(projectFileName_).dir();

    QFile file(dir.filePath(".tmp/" + QFileInfo(projectFileName_).completeBaseName() + ".md5"));
    if (!file.open(QIODevice::ReadOnly))
        return;

    QDataStream out(&file);
    out >> md5_;
}

void Application::saveMD5()
{
    QDir dir = QFileInfo(projectFileName_).dir();
    dir.mkdir(".tmp");

    QFile file(dir.filePath(".tmp/" + QFileInfo(projectFileName_).completeBaseName() + ".md5"));
    if (!file.open(QIODevice::WriteOnly))
        return;

    QDataStream out(&file);
    out << md5_;
}

extern "C"
{
#include <md5.h>
}

static bool md5_fromfile(const char* filename, unsigned char md5sum[16])
{
    FILE* f = fopen(filename, "rb");

    if (f == NULL)
        return false;

    md5_context ctx;
    unsigned char buf[1000];

    md5_starts( &ctx );

    int i;
    while( ( i = fread( buf, 1, sizeof( buf ), f ) ) > 0 )
    {
        md5_update( &ctx, buf, i );
    }

    md5_finish( &ctx, md5sum );

    fclose(f);

    return true;
}


std::vector<std::pair<QString, QString> > Application::updateMD5()
{
    std::vector<std::pair<QString, QString> > updated;

    std::vector<std::pair<QString, QString> > fileList = fileList_;

    QDir path(QFileInfo(projectFileName_).path());

    for (std::size_t i = 0; i < fileList.size(); ++i)
    {
        QString filename = fileList[i].second;
        QString absfilename = QDir::cleanPath(path.absoluteFilePath(filename));
        time_t mtime = fileLastModifiedTime(qPrintable(absfilename));

        QMap<QString, QPair<qint64, QByteArray> >::iterator iter = md5_.find(filename);

        if (iter == md5_.end() || mtime != iter.value().first)
        {
            QPair<qint64, QByteArray>& md5 = md5_[filename];

            md5.first = mtime;
            md5.second.resize(16);
            unsigned char* data = (unsigned char*)md5.second.data();
            md5_fromfile(qPrintable(absfilename), data);
//			updated = true;
            updated.push_back(fileList[i]);

            //qDebug() << "update md5: " + filename;
        }
    }

    return updated;
}

void Application::stop()
{
    fileQueue_.clear();
    client_->sendStop();
}

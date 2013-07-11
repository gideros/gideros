#define USE_LOCAL_SOCKETS 0

#include <QtCore/QCoreApplication>
#if USE_LOCAL_SOCKETS
#include <QLocalSocket>
#else
#include <QTcpSocket>
#endif
#include <QDebug>
#include <QStringList>
#include <QDir>
#include <QProcess>
#include <QMutex>
#include <QWaitCondition>

static bool sendData(const QByteArray &out, QByteArray *in)
{
#if USE_LOCAL_SOCKETS
    QLocalSocket socket;
    socket.connectToServer("gdrdeamon");
#else
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 15001);
#endif
    if (!socket.waitForConnected(1000))
        return false;

    qint32 size = out.size();
    socket.write((char*)&size, sizeof(qint32));
    socket.write(out);
    while (socket.bytesToWrite() && socket.waitForBytesWritten())
        ;

    while (socket.waitForReadyRead())
        in->append(socket.readAll());

    return true;
}

static void usage()
{
    fprintf(stderr, "Gideros Player Bridge v1.02\n\n");
    fprintf(stderr, "usage:\n");
    fprintf(stderr, "gdrbridge setip 127.0.0.1\n");
    fprintf(stderr, "gdrbridge play mygame.gproj\n");
    fprintf(stderr, "gdrbridge stop\n");
    fprintf(stderr, "gdrbridge isconnected\n");
    fprintf(stderr, "gdrbridge getlog\n");
    fprintf(stderr, "gdrbridge stopdeamon\n");
}

static void msleep(unsigned long msecs)
{
    QMutex mutex;
    mutex.lock();

    QWaitCondition waitCondition;
    waitCondition.wait(&mutex, msecs);

    mutex.unlock();
}

static bool isDeamonRunning()
{
#if USE_LOCAL_SOCKETS
    QLocalSocket socket;
    socket.connectToServer("gdrdeamon2");
#else
    QTcpSocket socket;
    socket.connectToHost("127.0.0.1", 15002);
#endif
    return socket.waitForConnected(1000);
}

static bool startDeamon(const QString &applicationDirPath)
{
    if (isDeamonRunning())
        return true;

    fprintf(stderr, "* daemon not running. starting it now on port 15001 *\n");
#if defined(Q_OS_WIN)
    QString program = QDir(applicationDirPath).absoluteFilePath("gdrdeamon.exe");
#else
    QString program = QDir(applicationDirPath).absoluteFilePath("gdrdeamon");
#endif
    QStringList arguments;
    QString workingDirectory = applicationDirPath;
    if (!QProcess::startDetached(program, arguments, workingDirectory))
    {
        fprintf(stderr, "* cannot run deamon *\n");
        return false;
    }

    msleep(100);

    for (int i = 0; i < 10; ++i)
    {
        if (isDeamonRunning())
        {
            fprintf(stderr, "* daemon started successfully *\n");
            msleep(100);
            return true;
        }

        msleep(100);
    }

    fprintf(stderr, "* cannot connect to deamon *\n");

    return false;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QStringList arguments = a.arguments();

    if (arguments.size() <= 1)
    {
        usage();
        return EXIT_FAILURE;
    }

    QByteArray out, in;
    QDataStream outstream(&out, QIODevice::WriteOnly);

    if (arguments[1] == "play")
    {
        if (arguments.size() <= 2)
        {
            usage();
            return EXIT_FAILURE;
        }

        outstream << QString("play") << QDir::current().absoluteFilePath(arguments[2]);
    }
    else if (arguments[1] == "stop")
    {
        outstream << QString("stop");
    }
    else if (arguments[1] == "setip")
    {
        if (arguments.size() <= 2)
        {
            usage();
            return EXIT_FAILURE;
        }

        outstream << QString("setip") << arguments[2];
    }
    else if (arguments[1] == "isconnected")
    {
        outstream << QString("isconnected");
    }
    else if (arguments[1] == "getlog")
    {
        outstream << QString("getlog");
    }
    else if (arguments[1] == "stopdeamon")
    {
        if (isDeamonRunning())
            outstream << QString("stopdeamon");
        else
        {
            fprintf(stderr, "* deamon not running *\n");
            return EXIT_FAILURE;
        }
    }
    else
    {
        usage();
        return EXIT_FAILURE;
    }

    if (arguments[1] != "stopdeamon")
    {
        if (!startDeamon(a.applicationDirPath()))
            return EXIT_FAILURE;
    }

    sendData(out, &in);
    QDataStream instream(in);

    while(!instream.atEnd())
    {
        QString line;
        instream >> line;
        printf("%s", line.toUtf8().data());
    }

    if (arguments[1] == "stopdeamon")
    {
        msleep(1000);
    }

    return 0;
}

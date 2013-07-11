#ifndef APPLICATION_H
#define APPLICATION_H

#define USE_LOCAL_SOCKETS 0

#include <QObject>
#include <deque>
#include <QString>
#include <QPair>
#include <projectproperties.h>
#include <QMap>
#include <QByteArray>
#include <dependencygraph.h>
#include <QStringList>
#include <QTime>

class QTimer;
class GiderosNetworkClient;
#if USE_LOCAL_SOCKETS
class QLocalServer;
#else
class QTcpServer;
#endif

class Application : public QObject
{
    Q_OBJECT
public:
    explicit Application(QObject *parent = 0);
    
signals:
    
public slots:

private slots:
    void newConnection();
    void ignoreConnection();

private slots:
    void connected();
    void disconnected();
    void dataReceived(const QByteArray& data);
    void ackReceived(unsigned int id);

private slots:
    void timer();

private:
    void play(const QString &fileName);
    void stop();

private:
    void loadMD5();
    void saveMD5();
    std::vector<std::pair<QString, QString> > updateMD5();

private:
    QTimer *timer_;
    GiderosNetworkClient *client_;
#if USE_LOCAL_SOCKETS
    QLocalServer *server_;
    QLocalServer *server2_;
#else
    QTcpServer *server_;
    QTcpServer *server2_;
#endif
    std::deque<QPair<QString, QString> > fileQueue_;
    bool isTransferring_;
    ProjectProperties properties_;
    std::vector<std::pair<QString, QString> > fileList_;
    QMap<QString, QPair<qint64, QByteArray> > md5_;
    QString projectFileName_;
    DependencyGraph dependencyGraph_;
    QStringList log_;
    QTime time_;
};

#endif // APPLICATION_H

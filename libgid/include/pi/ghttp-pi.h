#ifndef GHTTPPI_H
#define GHTTPPI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
class QAuthenticator;
class QNetworkProxy;
#include <deque>
#include <map>

#include <ghttp.h>

class HTTPManager : public QObject
{
    Q_OBJECT

public:
    HTTPManager();
    ~HTTPManager();

    g_id Get(const char* url, const ghttp_Header *header, gevent_Callback callback, void* udata);
    g_id Post(const char* url, const ghttp_Header *header, const void* data, size_t size, gevent_Callback callback, void* udata);
    g_id Delete(const char* url, const ghttp_Header *header, gevent_Callback callback, void* udata);
    g_id Put(const char* url, const ghttp_Header *header, const void* data, size_t size, gevent_Callback callback, void* udata);
    void Close(g_id id);
    void CloseAll();

private slots:
    void finished(QNetworkReply *reply);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    QNetworkAccessManager *manager_;

    struct NetworkReply
    {
        g_id id;
        gevent_Callback callback;
        void *udata;
    };
    std::map<QNetworkReply*, NetworkReply> map_;
};

#endif

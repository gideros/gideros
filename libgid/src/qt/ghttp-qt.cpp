#include <ghttp.h>
#include <ghttp-qt.h>

HTTPManager::HTTPManager()
{
    manager_ = new QNetworkAccessManager();
    connect(manager_, SIGNAL(finished(QNetworkReply*)),
            this,		SLOT(finished(QNetworkReply*)));
}

HTTPManager::~HTTPManager()
{
    CloseAll();
    delete manager_;
}

void HTTPManager::Close(g_id id)
{
    std::map<QNetworkReply*, NetworkReply>::iterator iter = map_.begin(), e = map_.end();
    for (; iter != e; ++iter)
    {
        if (iter->second.id == id)
        {
            iter->first->deleteLater();
            map_.erase(iter);
            break;
        }
    }

    gevent_RemoveEventsWithGid(id);
}

void HTTPManager::CloseAll()
{
    while (!map_.empty())
        Close(map_.begin()->second.id);
}

g_id HTTPManager::Get(const char *url, const ghttp_Header *header, gevent_Callback callback, void *udata)
{
    QNetworkRequest request(QUrl::fromEncoded(url));

    request.setRawHeader("User-Agent", "Gideros");

    if (header)
        for (; header->name; ++header)
            request.setRawHeader(QByteArray(header->name), QByteArray(header->value));

    QNetworkReply *reply = manager_->get(request);

    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this,	 SLOT(downloadProgress(qint64, qint64)));

    NetworkReply reply2;
    reply2.id = g_NextId();
    reply2.callback = callback;
    reply2.udata = udata;
    map_[reply] = reply2;

    return reply2.id;
}


g_id HTTPManager::Post(const char *url, const ghttp_Header *header, const void *data, size_t size, gevent_Callback callback, void *udata)
{
    QNetworkRequest request(QUrl::fromEncoded(url));

    request.setRawHeader("User-Agent", "Gideros");

    if (header)
        for (; header->name; ++header)
            request.setRawHeader(QByteArray(header->name), QByteArray(header->value));

    QNetworkReply *reply = manager_->post(request, QByteArray((char*)data, size));

    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this,	 SLOT(downloadProgress(qint64, qint64)));

    NetworkReply reply2;
    reply2.id = g_NextId();
    reply2.callback = callback;
    reply2.udata = udata;
    map_[reply] = reply2;

    return reply2.id;
}

g_id HTTPManager::Delete(const char *url, const ghttp_Header *header, gevent_Callback callback, void *udata)
{
    QNetworkRequest request(QUrl::fromEncoded(url));

    request.setRawHeader("User-Agent", "Gideros");

    if (header)
        for (; header->name; ++header)
            request.setRawHeader(QByteArray(header->name), QByteArray(header->value));

    QNetworkReply *reply = manager_->deleteResource(request);

    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this,	 SLOT(downloadProgress(qint64, qint64)));

    NetworkReply reply2;
    reply2.id = g_NextId();
    reply2.callback = callback;
    reply2.udata = udata;
    map_[reply] = reply2;

    return reply2.id;
}

g_id HTTPManager::Put(const char *url, const ghttp_Header *header, const void *data, size_t size, gevent_Callback callback, void *udata)
{
    QNetworkRequest request(QUrl::fromEncoded(url));

    request.setRawHeader("User-Agent", "Gideros");

    if (header)
        for (; header->name; ++header)
            request.setRawHeader(QByteArray(header->name), QByteArray(header->value));

    QNetworkReply *reply = manager_->put(request, QByteArray((char*)data, size));

    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this,	 SLOT(downloadProgress(qint64, qint64)));

    NetworkReply reply2;
    reply2.id = g_NextId();
    reply2.callback = callback;
    reply2.udata = udata;
    map_[reply] = reply2;

    return reply2.id;
}

void HTTPManager::finished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (map_.find(reply) == map_.end())
        return;

    if (reply->error() != QNetworkReply::NoError
        // a web page that returns 403/404 can still have content
        && reply->error() != QNetworkReply::ContentOperationNotPermittedError
        && reply->error() != QNetworkReply::ContentNotFoundError
        && reply->error() != QNetworkReply::UnknownContentError)
    {
        NetworkReply reply2 = map_[reply];

        ghttp_ErrorEvent *event = (ghttp_ErrorEvent*)malloc(sizeof(ghttp_ErrorEvent));

        gevent_EnqueueEvent(reply2.id, reply2.callback, GHTTP_ERROR_EVENT, event, 1, reply2.udata);
    }
    else
    {
        QByteArray bytes = reply->readAll();

        NetworkReply reply2 = map_[reply];

        ghttp_ResponseEvent *event = (ghttp_ResponseEvent*)malloc(sizeof(ghttp_ResponseEvent) + bytes.size());

        event->data = (char*)event + sizeof(ghttp_ResponseEvent);
        memcpy(event->data, bytes.constData(), bytes.size());
        event->size = bytes.size();

        QVariant httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (httpStatusCode.isValid())
            event->httpStatusCode = httpStatusCode.toInt();
        else
            event->httpStatusCode = -1;

        gevent_EnqueueEvent(reply2.id, reply2.callback, GHTTP_RESPONSE_EVENT, event, 1, reply2.udata);
    }

    map_.erase(reply);
}

void HTTPManager::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if (map_.find(reply) == map_.end())
        return;

    NetworkReply reply2 = map_[reply];

    ghttp_ProgressEvent* event = (ghttp_ProgressEvent*)malloc(sizeof(ghttp_ProgressEvent));
    event->bytesLoaded = bytesReceived;
    event->bytesTotal = bytesTotal;

    gevent_EnqueueEvent(reply2.id, reply2.callback, GHTTP_PROGRESS_EVENT, event, 1, reply2.udata);
}

static HTTPManager* s_manager = NULL;

extern "C" {

void ghttp_Init()
{
    s_manager = new HTTPManager();
}

void ghttp_Cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

g_id ghttp_Get(const char* url, const ghttp_Header *header, gevent_Callback callback, void* udata)
{
    return s_manager->Get(url, header, callback, udata);
}

g_id ghttp_Post(const char* url, const ghttp_Header *header, const void* data, size_t size, gevent_Callback callback, void* udata)
{
    return s_manager->Post(url, header, data, size, callback, udata);
}

g_id ghttp_Delete(const char* url, const ghttp_Header *header, gevent_Callback callback, void* udata)
{
    return s_manager->Delete(url, header, callback, udata);
}

g_id ghttp_Put(const char* url, const ghttp_Header *header, const void* data, size_t size, gevent_Callback callback, void* udata)
{
    return s_manager->Put(url, header, data, size, callback, udata);
}

void ghttp_Close(g_id id)
{
    s_manager->Close(id);
}

void ghttp_CloseAll()
{
    s_manager->CloseAll();
}

}

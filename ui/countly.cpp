#include "countly.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QUrl>
#include <QEventLoop>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <uid.h>

CountlyHelper::CountlyHelper()
{
    start();
}

void CountlyHelper::enqueue(const QString &url)
{
    mutex_.lock();
    queue_.enqueue(url);
    mutex_.unlock();

    semaphore_.release(1);
}

void CountlyHelper::run()
{
    QNetworkAccessManager manager;
    running_ = true;
    while (true)
    {
        mutex_.lock();
        if (!running_ && queue_.empty())
        {
            mutex_.unlock();
            break;
        }
        mutex_.unlock();

        semaphore_.acquire(1);

        QString url;
        mutex_.lock();
        if (!queue_.empty())
        {
            url = queue_.head();
            queue_.dequeue();
        }
        mutex_.unlock();

        if (!url.isNull())
        {
            QNetworkReply *reply = manager.get(QNetworkRequest(QUrl(url)));
            QEventLoop loop;
            QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
            loop.exec();
            delete reply;
        }
    }
}

void CountlyHelper::finish()
{
    mutex_.lock();
    running_ = false;
    mutex_.unlock();

    semaphore_.release(1);
}

Countly::Countly(const QString &version, int licenseType, QObject *parent) :
    QObject(parent)
{
    appKey_ = "bafdab0db44c28d6e428df5afeb6776db81d7fba";
    version_ = version;
    licenseType_ = licenseType;
    uid_ = QString::fromStdString(g_uid());
#if defined(Q_OS_WIN)
    os_ = "Windows";
#elif defined(Q_OS_MAC)
    os_ = "Mac";
#else
    os_ = "Unknown";
#endif
}

void Countly::beginSession()
{
    QString licenseType = "unknown";
    switch (licenseType_)
    {
    case -1:
        licenseType = "none";
        break;
    case 1:
        licenseType = "free";
        break;
    case 2:
        licenseType = "indie";
        break;
    case 3:
        licenseType = "pro";
        break;
    }

    QString metrics = QString("{\"_os\":\"%1\",\"_app_version\":\"%2\",\"_carrier\":\"%3\"}").arg(os_).arg(version_).arg(licenseType);
    helper_.enqueue(QString("http://stats.giderosmobile.com/i?app_key=%1&device_id=%2&begin_session=1&metrics=%3").arg(appKey_).arg(uid_).arg(metrics));
    timer_.start();
}

void Countly::endSession()
{
    int sec = timer_.elapsed() / 1000;
    helper_.enqueue(QString("http://stats.giderosmobile.com/i?app_key=%1&device_id=%2&end_session=1&session_duration=%3").arg(appKey_).arg(uid_).arg(sec));
}

void Countly::projectCreated()
{
    QString events = "[{\"key\":\"projects_created\",\"count\":1}]";
    helper_.enqueue(QString("http://stats.giderosmobile.com/i?app_key=%1&device_id=%2&events=%3").arg(appKey_).arg(uid_).arg(events));
}

void Countly::projectExported()
{
    QString events = "[{\"key\":\"projects_exported\",\"count\":1}]";
    helper_.enqueue(QString("http://stats.giderosmobile.com/i?app_key=%1&device_id=%2&events=%3").arg(appKey_).arg(uid_).arg(events));
}

void Countly::projectPlayed()
{
    QString events = "[{\"key\":\"projects_played\",\"count\":1}]";
    helper_.enqueue(QString("http://stats.giderosmobile.com/i?app_key=%1&device_id=%2&events=%3").arg(appKey_).arg(uid_).arg(events));
}

void Countly::waitForFinished(unsigned long time)
{
    helper_.finish();
    helper_.wait(time);
}

#ifndef COUNTLY_H
#define COUNTLY_H

#include <QObject>
#include <QThread>
#include <QQueue>
#include <QString>
#include <QMutex>
#include <QSemaphore>
#include <QElapsedTimer>

class CountlyHelper : public QThread
{
    Q_OBJECT

public:
    CountlyHelper();

    void enqueue(const QString &url);
    void finish();

protected:
    virtual void run();

private:
    QQueue<QString> queue_;
    volatile bool running_;
    QSemaphore semaphore_;
    QMutex mutex_;
};

class Countly : public QObject
{
    Q_OBJECT
public:
    explicit Countly(const QString &version, int licenseType, QObject *parent = NULL);

    void beginSession();
    void endSession();

    void projectCreated();
    void projectExported();
    void projectPlayed();

    void waitForFinished(unsigned long time = ULONG_MAX);

signals:
    
public slots:

private:
    CountlyHelper helper_;
    QElapsedTimer timer_;
    QString appKey_;
    QString version_;
    int licenseType_;
    QString uid_;
    QString os_;
};

#endif // COUNTLY_H

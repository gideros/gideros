#ifndef MEDIA_H
#define MEDIA_H

#include "gideros.h"
#include <QObject>
#include <QCameraInfo>


typedef struct gmedia_ReceivedEvent
{
    const char* path;
} gmedia_ReceivedEvent;

enum
{
    GMEDIA_RECEIVED_EVENT,
    GMEDIA_CANCELED_EVENT,
    GMEDIA_COMPLETED_EVENT
};

class GMEDIA : QObject
{
	Q_OBJECT
public:
	GMEDIA();
	virtual ~GMEDIA();
	bool isCameraAvailable();
    QCamera* camera;
	void takePicture();
    void takeScreenshot();
	void getPicture();
    void savePicture(const char* path);
    void playVideo(const char* path, bool force);
    QString getAppPath();
    void onMediaReceived(const char* path);
    void onMediaCompleted();
	void onMediaCanceled();
	g_id addCallback(gevent_Callback callback, void *udata);
	void removeCallback(gevent_Callback callback, void *udata);
	void removeCallbackWithGid(g_id gid);
private:
	static void callback_s(int type, void *event, void *udata);
	void callback(int type, void *event);
private slots:
    void pictureSaved(int id, const QString & fileName);

private:
	gevent_CallbackList callbackList_;

private:
    g_id gid_;
};


#ifdef __cplusplus
extern "C" {
#endif

void gmedia_init();
void gmedia_cleanup();

int gmedia_isCameraAvailable();
void gmedia_takePicture();
void gmedia_takeScreenshot();
void gmedia_getPicture();
void gmedia_savePicture(const char* path);
void gmedia_playVideo(const char* path, int force);

g_id gmedia_addCallback(gevent_Callback callback, void *udata);
void gmedia_removeCallback(gevent_Callback callback, void *udata);
void gmedia_removeCallbackWithGid(g_id gid);

#ifdef __cplusplus
}
#endif

#endif

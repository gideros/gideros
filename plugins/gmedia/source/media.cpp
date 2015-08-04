#include <media.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QCameraInfo>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QVideoWidget>
#include <QCameraImageCapture>
#include <QString>
#include <QFileDialog>
#include <QPixmap>
#include <QDateTime>
#include <QObject>

	GMEDIA::GMEDIA()
	{
		gid_ = g_NextId();
		camera=NULL;
	}

	GMEDIA::~GMEDIA()
	{

		gevent_RemoveEventsWithGid(gid_);
    }
	
	bool GMEDIA::isCameraAvailable()
	{
        if (QCameraInfo::availableCameras().count() > 0)
            return true;
        else
            return false;
    }
	
	void GMEDIA::takePicture()
	{
		if (!camera)
		{
        QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
        foreach (const QCameraInfo &cameraInfo, cameras) {
            camera = new QCamera(cameraInfo);
            break;
        }
		}

        QCameraImageCapture* imageCapture = new QCameraImageCapture(camera);

        camera->setCaptureMode(QCamera::CaptureStillImage);
        camera->start(); // Viewfinder frames start flowing

        //on half pressed shutter button
        camera->searchAndLock();

        //on shutter button pressed
        QDateTime createdDate = QDateTime::currentDateTime();
        QString format = "jpg";
        QString fileName = getAppPath() + "/" + createdDate.toString("yyyyMMdd_HHmmss") + "_gideros." + format;

        connect(imageCapture, SIGNAL(imageSaved(int, const QString &)),
                         this,  SLOT(pictureSaved(int, const QString &)));
        imageCapture->capture(fileName);
	}

    void GMEDIA::takeScreenshot()
    {
        QDateTime createdDate = QDateTime::currentDateTime();
        //QPixmap originalPixmap = QPixmap::grabWindow(QApplication::activeWindow()->winId());
        QWidget *widget = QApplication::activeWindow();
        QPixmap originalPixmap = QPixmap::grabWindow(QApplication::desktop()->winId());
        QPixmap windowPixmap = originalPixmap.copy(widget->geometry().x(), widget->geometry().y(), widget->width(), widget->height());
        QString format = "png";
        QString fileName = getAppPath() + "/" + createdDate.toString("yyyyMMdd_HHmmss") + "_gideros." + format;
        windowPixmap.save(fileName, format.toStdString().c_str());
        onMediaReceived(fileName.toStdString().c_str());
    }
	
	void GMEDIA::getPicture()
	{
        QString fileName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),"",QObject::tr("Images (*.png *.jpeg *.jpg)"));
        if(fileName.isNull())
            onMediaCanceled();
        else{
            QFileInfo info(fileName);
            QDateTime createdDate = QDateTime::currentDateTime();
            QString destName = getAppPath() + "/" + createdDate.toString("yyyyMMdd_HHmmss") + "_gideros." + info.suffix();
            if(QFile::exists(destName))
            {
                QFile::remove(destName);
            }
            QFile::copy(fileName, destName);
            onMediaReceived(destName.toStdString().c_str());
        }
	}
	
    void GMEDIA::savePicture(const char* path)
	{
        QFileInfo info(path);
        QString format = info.suffix();
        QString initialPath = getAppPath() + "/" + info.fileName();

        QString fileName = QFileDialog::getSaveFileName(0, QObject::tr("Save As"),
                                        initialPath,
                                        QObject::tr("%1 Files (*.%2);;All Files (*)")
                                        .arg(format.toUpper())
                                        .arg(format));
        if (!fileName.isEmpty())
        {
            if(QFile::exists(fileName))
            {
                QFile::remove(fileName);
            }

            QFile::copy(path, fileName);
        }
	}

    void GMEDIA::playVideo(const char* path, bool force)
    {
        QMediaPlayer* player = new QMediaPlayer;

        QMediaPlaylist* playlist = new QMediaPlaylist(player);
        playlist->addMedia(QUrl(path));

        QVideoWidget* videoWidget = new QVideoWidget;
        player->setVideoOutput(videoWidget);

        videoWidget->show();
        playlist->setCurrentIndex(1);
        player->play();
    }

    QString GMEDIA::getAppPath(){
        QDir dir = QDir::temp();
        dir.mkdir("gideros");
        dir.cd("gideros");
        dir.mkdir("mediafiles");
        dir.cd("mediafiles");
        return dir.absolutePath();
    }
	
    void GMEDIA::onMediaReceived(const char* path)
	{

		gmedia_ReceivedEvent *event = (gmedia_ReceivedEvent*)gevent_CreateEventStruct1(
			sizeof(gmedia_ReceivedEvent),
            offsetof(gmedia_ReceivedEvent, path), path);
		gevent_EnqueueEvent(gid_, callback_s, GMEDIA_RECEIVED_EVENT, event, 1, this);
	}

    void GMEDIA::onMediaCompleted()
    {
        gevent_EnqueueEvent(gid_, callback_s, GMEDIA_COMPLETED_EVENT, NULL, 1, this);
    }
	
	void GMEDIA::onMediaCanceled()
    {
		gevent_EnqueueEvent(gid_, callback_s, GMEDIA_CANCELED_EVENT, NULL, 1, this);
	}
	
	g_id GMEDIA::addCallback(gevent_Callback callback, void *udata)
	{
		return callbackList_.addCallback(callback, udata);
	}
	void GMEDIA::removeCallback(gevent_Callback callback, void *udata)
	{
		callbackList_.removeCallback(callback, udata);
	}
	void GMEDIA::removeCallbackWithGid(g_id gid)
	{
		callbackList_.removeCallbackWithGid(gid);
	}

	void GMEDIA::callback_s(int type, void *event, void *udata)
	{
		((GMEDIA*)udata)->callback(type, event);
	}

	void GMEDIA::callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

    void GMEDIA::pictureSaved(int id, const QString & fileName)
    {
        //on shutter button released
        camera->unlock();
        camera->stop();
        onMediaReceived(fileName.toStdString().c_str());
    }


static GMEDIA *s_gmedia = NULL;

extern "C" {

void gmedia_init()
{
    s_gmedia = new GMEDIA;
}

void gmedia_cleanup()
{
    delete s_gmedia;
    s_gmedia = NULL;
}

int gmedia_isCameraAvailable()
{
    return s_gmedia->isCameraAvailable();
}

void gmedia_takePicture(){
    s_gmedia->takePicture();
}

void gmedia_takeScreenshot(){
    s_gmedia->takeScreenshot();
}

void gmedia_getPicture(){
    s_gmedia->getPicture();
}

void gmedia_savePicture(const char* path){
    s_gmedia->savePicture(path);
}

void gmedia_playVideo(const char* path, int force){
    s_gmedia->playVideo(path, force);
}

g_id gmedia_addCallback(gevent_Callback callback, void *udata)
{
    return s_gmedia->addCallback(callback, udata);
}

void gmedia_removeCallback(gevent_Callback callback, void *udata)
{
    s_gmedia->removeCallback(callback, udata);
}

void gmedia_removeCallbackWithGid(g_id gid)
{
    s_gmedia->removeCallbackWithGid(gid);
}

}

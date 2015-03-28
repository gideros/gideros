#include "media.h"
#import "GMediaClass.h"

class GMEDIA
{
public:
	GMEDIA()
	{
		gid_ = g_NextId();
        gm = [[GMediaClass alloc] init];
	}

	~GMEDIA()
	{

		gevent_RemoveEventsWithGid(gid_);
        [gm deinit];
        [gm release];
        gm = nil;
    }
	
	bool isCameraAvailable()
	{
        return [gm isCameraAvailable];
    }
	
	void takePicture()
	{
        [gm takePicture];
	}
    
    void takeScreenshot()
	{
        [gm takeScreenshot];
	}
	
	void getPicture()
	{
        [gm getPicture];
	}
	
	void savePicture(const char* path)
	{
        [gm addToGallery:[NSString stringWithUTF8String:path]];
	}
    
    void playVideo(const char* path, bool force)
	{
		[gm playVideo:[NSString stringWithUTF8String:path] shouldForce:force];
	}
	
    void onMediaReceived(const char* path)
	{

		gmedia_ReceivedEvent *event = (gmedia_ReceivedEvent*)gevent_CreateEventStruct1(
			sizeof(gmedia_ReceivedEvent),
            offsetof(gmedia_ReceivedEvent, path), path);
		gevent_EnqueueEvent(gid_, callback_s, GMEDIA_RECEIVED_EVENT, event, 1, this);
	}
	
	void onMediaCanceled()
    {
		gevent_EnqueueEvent(gid_, callback_s, GMEDIA_CANCELED_EVENT, NULL, 1, this);
	}
    
    void onMediaCompleted()
	{
		gevent_EnqueueEvent(gid_, callback_s, GMEDIA_COMPLETED_EVENT, NULL, 1, this);
	}
	
	g_id addCallback(gevent_Callback callback, void *udata)
	{
		return callbackList_.addCallback(callback, udata);
	}
	void removeCallback(gevent_Callback callback, void *udata)
	{
		callbackList_.removeCallback(callback, udata);
	}
	void removeCallbackWithGid(g_id gid)
	{
		callbackList_.removeCallbackWithGid(gid);
	}

private:
	static void callback_s(int type, void *event, void *udata)
	{
		((GMEDIA*)udata)->callback(type, event);
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
    g_id gid_;
    GMediaClass *gm;
};


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


int gmedia_isCameraAvailable(){
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
    
void gmedia_onMediaReceive(const char* path){
    s_gmedia->onMediaReceived(path);
}

void gmedia_onMediaCanceled(){
    s_gmedia->onMediaCanceled();
}
    
void gmedia_onMediaCompleted(){
    s_gmedia->onMediaCompleted();
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

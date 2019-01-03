#include "ads.h"
#include <stdlib.h>
#include <glog.h>
#include <emscripten.h>

#include "cJSON.h"
extern "C" cJSON *JSCall(const char *mtd, cJSON *args);
extern "C" void JSCallV(const char *mtd, cJSON *args);

class GAds
{
public:
	GAds()
	{
		gid_ = g_NextId();
		
		JSCallV("GiderosAds.Init",NULL);
	}

	~GAds()
	{
		JSCallV("GiderosAds.Deinit",NULL);
		
		gevent_RemoveEventsWithGid(gid_);
	}
	
	void init(const char *ad)
	{
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
		JSCallV("GiderosAds.Inititialize",args);
	}
	
	void destroy(const char *ad)
	{
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
		JSCallV("GiderosAds.Destroy",args);
	}
	
	void setKey(const char *ad, gads_Parameter *params)
	{
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
    	cJSON *perms=cJSON_CreateObject();
    	cJSON_AddItemToArray(args,perms);
	    while (params->value)
	    {
	     	cJSON_AddItemToArray(perms,cJSON_CreateString(params->value));
	     	params++;
	    }

		JSCallV("GiderosAds.SetKey",args);
	}
	
	void loadAd(const char *ad, gads_Parameter *params)
	{
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
    	cJSON *perms=cJSON_CreateObject();
    	cJSON_AddItemToArray(args,perms);
	    while (params->value)
	    {
	     	cJSON_AddItemToArray(perms,cJSON_CreateString(params->value));
	     	params++;
	    }

		JSCallV("GiderosAds.LoadAd",args);
	}
	
	void showAd(const char *ad, gads_Parameter *params)
	{
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
    	cJSON *perms=cJSON_CreateObject();
    	cJSON_AddItemToArray(args,perms);
	    while (params->value)
	    {
	     	cJSON_AddItemToArray(perms,cJSON_CreateString(params->value));
	     	params++;
	    }

		JSCallV("GiderosAds.ShowAd",args);
	}
	
	void hideAd(const char *ad, const char *type)
	{
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
    	cJSON_AddItemToArray(args,cJSON_CreateString(type));
		JSCallV("GiderosAds.HideAd",args);
	}
	
	void enableTesting(const char *ad)
	{
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
		JSCallV("GiderosAds.Enabletesting",args);
	}
	
	void setAlignment(const char *ad, const char *hor, const char *ver)
	{
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
    	cJSON_AddItemToArray(args,cJSON_CreateString(hor));
    	cJSON_AddItemToArray(args,cJSON_CreateString(ver));
		JSCallV("GiderosAds.SetAlignment",args);
	}
	
	void setX(const char *ad, int x)
	{
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
    	cJSON_AddItemToArray(args,cJSON_CreateNumber(x));
		JSCallV("GiderosAds.SetX",args);
	}
	
	void setY(const char *ad, int y)
	{
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
    	cJSON_AddItemToArray(args,cJSON_CreateNumber(y));
		JSCallV("GiderosAds.SetY",args);
	}
	
	int getX(const char *ad)
	{
    	int v;
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
		cJSON *r=JSCall("GiderosAds.GetX",args);
		v=r->valueint;
		cJSON_Delete(r);
		return v;
	}
	
	int getY(const char *ad)
	{
    	int v;
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
		cJSON *r=JSCall("GiderosAds.GetY",args);
		v=r->valueint;
		cJSON_Delete(r);
		return v;
	}
	
	int getWidth(const char *ad)
	{
    	int v;
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
		cJSON *r=JSCall("GiderosAds.GetWidth",args);
		v=r->valueint;
		cJSON_Delete(r);
		return v;
	}
	
	int getHeight(const char *ad)
	{
    	int v;
    	cJSON *args=cJSON_CreateArray();
    	cJSON_AddItemToArray(args,cJSON_CreateString(ad));
		cJSON *r=JSCall("GiderosAds.GetHeight",args);
		v=r->valueint;
		cJSON_Delete(r);
		return v;
	}
	
	void onAdReceived(const char *ad, const char *type)
	{
		gads_SimpleEvent *event = (gads_SimpleEvent*)gevent_CreateEventStruct2(
			sizeof(gads_SimpleEvent),
			offsetof(gads_SimpleEvent, ad), ad,
			offsetof(gads_SimpleEvent, type), type);

		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_RECEIVED_EVENT, event, 1, this);
	}
	
	void onAdFailed(const char *ad,const char *type,const char *error)
	{
		gads_AdFailedEvent *event = (gads_AdFailedEvent*)gevent_CreateEventStruct3(
			sizeof(gads_AdFailedEvent),
			offsetof(gads_AdFailedEvent, ad), ad,
			offsetof(gads_AdFailedEvent, type), type,
			offsetof(gads_AdFailedEvent, error), error);

		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_FAILED_EVENT, event, 1, this);
	}
	
	void onAdActionBegin(const char *ad, const char *type)
	{
		gads_SimpleEvent *event = (gads_SimpleEvent*)gevent_CreateEventStruct2(
			sizeof(gads_SimpleEvent),
			offsetof(gads_SimpleEvent, ad), ad,
			offsetof(gads_SimpleEvent, type), type);

		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_ACTION_BEGIN_EVENT, event, 1, this);
	}
	
	void onAdActionEnd(const char *ad, const char *type)
	{
		gads_SimpleEvent *event = (gads_SimpleEvent*)gevent_CreateEventStruct2(
			sizeof(gads_SimpleEvent),
			offsetof(gads_SimpleEvent, ad), ad,
			offsetof(gads_SimpleEvent, type), type);

		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_ACTION_END_EVENT, event, 1, this);
	}
	
	void onAdDismissed(const char *ad, const char *type)
	{
		gads_SimpleEvent *event = (gads_SimpleEvent*)gevent_CreateEventStruct2(
			sizeof(gads_SimpleEvent),
			offsetof(gads_SimpleEvent, ad), ad,
			offsetof(gads_SimpleEvent, type), type);

		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_DISMISSED_EVENT, event, 1, this);
	}
	
	void onAdDisplayed(const char *ad, const char *type)
	{
		gads_SimpleEvent *event = (gads_SimpleEvent*)gevent_CreateEventStruct2(
            sizeof(gads_SimpleEvent),
            offsetof(gads_SimpleEvent, ad), ad,
            offsetof(gads_SimpleEvent, type), type);
			
		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_DISPLAYED_EVENT, event, 1, this);
	}
	
	void onAdError(const char *ad, const char *error)
	{
		gads_AdErrorEvent *event = (gads_AdErrorEvent*)gevent_CreateEventStruct2(
			sizeof(gads_AdErrorEvent),
			offsetof(gads_AdErrorEvent, ad), ad,
			offsetof(gads_AdErrorEvent, error), error);

		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_ERROR_EVENT, event, 1, this);
	}
	
	void onAdInitialized(const char *ad)
	{
		gads_SimpleEvent *event = (gads_SimpleEvent*)gevent_CreateEventStruct2(
            sizeof(gads_SimpleEvent),
            offsetof(gads_SimpleEvent, ad), ad,
            offsetof(gads_SimpleEvent, type), "");
			
		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_INITIALIZED_EVENT, event, 1, this);
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
		((GAds*)udata)->callback(type, event);
	}

	void callback(int type, void *event)
	{
		callbackList_.dispatchEvent(type, event);
	}

private:
	gevent_CallbackList callbackList_;

private:
	g_id gid_;
};

static GAds *s_ads = NULL;

extern "C" {

int gads_isAvailable()
{
	return 1;
}

void gads_init()
{
	s_ads = new GAds;
}

void gads_cleanup()
{
	if(s_ads)
	{
		delete s_ads;
		s_ads = NULL;
	}
}

void gads_initialize(const char *ad)
{
	if(s_ads)
	{
		s_ads->init(ad);
	}
}

void gads_destroy(const char *ad)
{
	if(s_ads != NULL)
	{
		s_ads->destroy(ad);
	}
}

void gads_setKey(const char *ad, gads_Parameter *params)
{
	if(s_ads)
	{
		s_ads->setKey(ad, params);
	}
}

void gads_loadAd(const char *ad, gads_Parameter *params)
{
	if(s_ads)
	{
		s_ads->loadAd(ad, params);
	}
}

void gads_showAd(const char *ad, gads_Parameter *params)
{
	if(s_ads)
	{
		s_ads->showAd(ad, params);
	}
}

void gads_hideAd(const char *ad, const char *type)
{
	if(s_ads)
	{
		s_ads->hideAd(ad, type);
	}
}

void gads_enableTesting(const char *ad)
{
	if(s_ads)
	{
		s_ads->enableTesting(ad);
	}
}

void gads_setAlignment(const char *ad, const char *hor, const char *ver)
{
	if(s_ads)
	{
		s_ads->setAlignment(ad, hor, ver);
	}
}

void gads_setX(const char *ad, int x)
{
	if(s_ads)
	{
		s_ads->setX(ad, x);
	}
}

void gads_setY(const char *ad, int y)
{
	if(s_ads)
	{
		s_ads->setY(ad, y);
	}
}

int gads_getX(const char *ad)
{
	return s_ads->getX(ad);
}

int gads_getY(const char *ad)
{
	return s_ads->getY(ad);
}

int gads_getWidth(const char *ad)
{
	return s_ads->getWidth(ad);
}

int gads_getHeight(const char *ad)
{
	return s_ads->getHeight(ad);
}

g_id gads_addCallback(gevent_Callback callback, void *udata)
{
	return s_ads->addCallback(callback, udata);
}

void gads_removeCallback(gevent_Callback callback, void *udata)
{
	if(s_ads)
	{
		s_ads->removeCallback(callback, udata);
	}
}

void gads_removeCallbackWithGid(g_id gid)
{
	if(s_ads)
	{
		s_ads->removeCallbackWithGid(gid);
	}
}

void gads_onAdReceived(const char *ad, const char *type)
{
	if(s_ads)
	 s_ads->onAdReceived(ad,type);
}

void gads_onAdFailed(const char *ad, const char *type, const char *error)
{
	if(s_ads)
	 s_ads->onAdFailed(ad,type,error);
}

void gads_onAdActionBegin(const char *ad, const char *type)
{
	if(s_ads)
		 s_ads->onAdActionBegin(ad,type);
}

void gads_onAdActionEnd(const char *ad, const char *type)
{
	if(s_ads)
		 s_ads->onAdActionEnd(ad,type);
}

void gads_onAdDismissed(const char *ad, const char *type)
{
	if(s_ads)
		 s_ads->onAdDismissed(ad,type);
}

void gads_onAdDisplayed(const char *ad, const char *type)
{
	if(s_ads)
		 s_ads->onAdDisplayed(ad,type);
}

void gads_onAdInitialized(const char *ad)
{
	if(s_ads)
		 s_ads->onAdInitialized(ad);
}

void gads_onAdError(const char *ad, const char *error)
{
	if(s_ads)
		 s_ads->onAdError(ad, error);
}

}

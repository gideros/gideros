#include "ads.h"
#include <stdlib.h>
#include <glog.h>
#import "AdsClass.h"

class GAds
{
public:
	GAds()
	{
		gid_ = g_NextId();
        [AdsClass init];
	}

	~GAds()
	{
        [AdsClass cleanup];
		gevent_RemoveEventsWithGid(gid_);
	}
	
	void init(const char *ad)
	{
		[AdsClass initialize:[NSString stringWithUTF8String:ad]];
	}
	
	void destroy(const char *ad)
	{
		[AdsClass destroy:[NSString stringWithUTF8String:ad]];
	}
    
    int hasConnection()
    {
        return 	[AdsClass hasConnection];
    }
	
	void setKey(const char *ad, gads_Parameter *params)
	{
        NSMutableArray *arr = [NSMutableArray array];
		while (params->value)
		{
            [arr addObject:[NSString stringWithUTF8String:params->value]];
			++params;
		}
		[AdsClass setKey:[NSString stringWithUTF8String:ad] with:arr];
	}
    
    void loadAd(const char *ad, gads_Parameter *params)
	{
        NSMutableArray *arr = [NSMutableArray array];
		while (params->value)
		{
			[arr addObject:[NSString stringWithUTF8String:params->value]];
			++params;
		}
		[AdsClass loadAd:[NSString stringWithUTF8String:ad] with:arr];
	}
	
	void showAd(const char *ad, gads_Parameter *params)
	{
        NSMutableArray *arr = [NSMutableArray array];
		while (params->value)
		{
			[arr addObject:[NSString stringWithUTF8String:params->value]];
			++params;
		}
		[AdsClass showAd:[NSString stringWithUTF8String:ad] with:arr];
	}
	
	void hideAd(const char *ad, const char *type)
	{
		[AdsClass hideAd:[NSString stringWithUTF8String:ad] with:[NSString stringWithUTF8String:type]];
	}
    
    void enableTesting(const char *ad)
	{
		[AdsClass enableTesting:[NSString stringWithUTF8String:ad]];
	}
	
	void setAlignment(const char *ad, const char *hor, const char *verr)
	{
        [AdsClass setAlignment:[NSString stringWithUTF8String:ad] with:[NSString stringWithUTF8String:hor] andWith:[NSString stringWithUTF8String:verr]];
	}
    
    void setX(const char *ad, int x)
	{
        [AdsClass setX:[NSString stringWithUTF8String:ad] with:x];
	}
	
	void setY(const char *ad, int y)
	{
        [AdsClass setY:[NSString stringWithUTF8String:ad] with:y];
	}
	
	int getX(const char *ad)
	{
        return [AdsClass getX:[NSString stringWithUTF8String:ad]];
	}
	
	int getY(const char *ad)
	{
        return [AdsClass getY:[NSString stringWithUTF8String:ad]];
	}
	
	int getWidth(const char *ad)
	{
        return [AdsClass getWidth:[NSString stringWithUTF8String:ad]];
	}
	
	int getHeight(const char *ad)
	{
        return [AdsClass getHeight:[NSString stringWithUTF8String:ad]];
	}
	
	void onAdsReady(const char* ad, int state)
	{
		gads_ReadyEvent* event = (gads_ReadyEvent*)gevent_CreateEventStruct1(
			sizeof(gads_ReadyEvent),
			offsetof(gads_ReadyEvent, ad), ad);
		event->state = state;
		gevent_EnqueueEvent(gid_, callback_s, GADS_ADS_READY_EVENT, event, 1, this);
	}
	
	void onAdReceived(const char *ad, const char *type)
	{
		gads_SimpleEvent *event = (gads_SimpleEvent*)gevent_CreateEventStruct2(
            sizeof(gads_SimpleEvent),
            offsetof(gads_SimpleEvent, ad), ad,
            offsetof(gads_SimpleEvent, type), type);
		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_RECEIVED_EVENT, event, 1, this);
	}
	
	void onAdFailed(const char *ad, const char *error, const char *type)
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
	
	void onAdRewarded(const char *ad, const char *type, int amount)
	{
		gads_RewardEvent *event = (gads_RewardEvent*)gevent_CreateEventStruct2(
            sizeof(gads_RewardEvent),
            offsetof(gads_RewardEvent, ad), ad,
            offsetof(gads_RewardEvent, type), type);
		event->amount=amount;
		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_REWARDED_EVENT, event, 1, this);
	}

	void onAdError(const char *ad, const char *error)
	{
		gads_AdErrorEvent *event = (gads_AdErrorEvent*)gevent_CreateEventStruct2(
			sizeof(gads_AdErrorEvent),
			offsetof(gads_AdErrorEvent, ad), ad,
			offsetof(gads_AdErrorEvent, error), error);

		gevent_EnqueueEvent(gid_, callback_s, GADS_AD_ERROR_EVENT, event, 1, this);
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
    
int gads_hasConnection()
{
    if(s_ads)
    {
        return s_ads->hasConnection();
    }
    return 0;
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

bool gads_hasProvider(const char *ad)
{
    return [AdsClass hasProvider:[NSString stringWithUTF8String:ad]];
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
    	
void gads_adReceived(const char *ad, const char *type){
    if(s_ads)
	{
        s_ads->onAdReceived(ad, type);
    }
}
    
void gads_adFailed(const char *ad, const char *error, const char *type){
    if(s_ads)
	{
        s_ads->onAdFailed(ad, error, type);
    }
}
    
void gads_adActionBegin(const char *ad, const char *type){
    if(s_ads)
	{
        s_ads->onAdActionBegin(ad, type);
    }
}
    
void gads_adActionEnd(const char *ad, const char *type){
    if(s_ads)
	{
        s_ads->onAdActionEnd(ad, type);
    }
}
    
void gads_adDismissed(const char *ad, const char *type){
    if(s_ads)
	{
        s_ads->onAdDismissed(ad, type);
    }
}
    
void gads_adDisplayed(const char *ad, const char *type){
    if(s_ads)
    {
        s_ads->onAdDisplayed(ad, type);
    }
}
    
void gads_adRewarded(const char *ad, const char *type, int amount){
    if(s_ads)
    {
        s_ads->onAdRewarded(ad, type, amount);
    }
}

void gads_adsReady(const char *ad, int state){
    if(s_ads)
    {
        s_ads->onAdsReady(ad, state);
    }
}

void gads_adError(const char *ad, const char *error){
    if(s_ads)
	{
        s_ads->onAdError(ad, error);
    }
}

}

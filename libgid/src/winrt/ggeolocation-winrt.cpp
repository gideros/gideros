#include "ggeolocation.h"
#include <ppltasks.h>
using namespace Windows::Devices::Geolocation;
using namespace Windows::Foundation;
static Geolocator^ gps=nullptr;
static EventRegistrationToken gps_event;
static bool gps_en = false;
static gevent_CallbackList callbackList_;
static g_id gid_;
static void callback_s(int type, void *event, void *udata)
{
	callbackList_.dispatchEvent(type, event);
}


static void gps_update(Geolocator^ gl,
	PositionChangedEventArgs^ pos)
{
	ggeolocation_LocationUpdateEvent *event = (ggeolocation_LocationUpdateEvent*)malloc(sizeof(ggeolocation_LocationUpdateEvent));
	event->latitude = pos->Position->Coordinate->Latitude;
	event->longitude = pos->Position->Coordinate->Longitude;
	event->altitude = pos->Position->Coordinate->Altitude->Value;

	gevent_EnqueueEvent(gid_, callback_s, GGEOLOCATION_LOCATION_UPDATE_EVENT, event, 1, NULL);
}

#ifdef __cplusplus
extern "C" {
#endif

void ggeolocation_init()
{
	gps = ref new Geolocator();
	gps->MovementThreshold = 0;
	gps->ReportInterval = 1;
	gid_ = g_NextId();
}

void ggeolocation_cleanup()
{
	ggeolocation_stopUpdatingLocation();
	gevent_RemoveEventsWithGid(gid_);
}

int ggeolocation_isAvailable()
{
    return (gps->LocationStatus==PositionStatus::Ready)|| (gps->LocationStatus == PositionStatus::Initializing) || (gps->LocationStatus == PositionStatus::NotInitialized) || (gps->LocationStatus == PositionStatus::NoData);
}

int ggeolocation_isHeadingAvailable()
{
    return 0;
}

void ggeolocation_setAccuracy(double accuracy)
{	
	gps->DesiredAccuracyInMeters= ref new Platform::Box<unsigned int>(accuracy);
}

double ggeolocation_getAccuracy()
{
    return gps->DesiredAccuracyInMeters->Value;
}

void ggeolocation_setThreshold(double threshold)
{
	gps->MovementThreshold = threshold;
}

double ggeolocation_getThreshold()
{
    return gps->MovementThreshold;
}

void ggeolocation_startUpdatingLocation()
{
	if (!gps_en)
	{
		gps_en = true;
		gps_event = (gps->PositionChanged += ref new  TypedEventHandler<Geolocator^,
			PositionChangedEventArgs^>(gps_update));
	}
}

void ggeolocation_stopUpdatingLocation()
{
	if (gps_en)
	{
		gps->PositionChanged -= gps_event;
		gps_en = false;
	}
}

void ggeolocation_startUpdatingHeading()
{

}

void ggeolocation_stopUpdatingHeading()
{

}

g_id ggeolocation_addCallback(gevent_Callback callback, void *udata)
{
	return callbackList_.addCallback(callback, udata);
}

void ggeolocation_removeCallback(gevent_Callback callback, void *udata)
{
	callbackList_.removeCallback(callback, udata);
}

void ggeolocation_removeCallbackWithGid(g_id gid)
{
	callbackList_.removeCallbackWithGid(gid);
}

#ifdef __cplusplus
}
#endif

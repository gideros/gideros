#if TARGET_OS_TV == 0
#include <ggeolocation.h>
#import <CoreLocation/CoreLocation.h>
#include <set>
#include <gevent.h>
#include <vector>
#include <map>

class GGGeolocationManager;

@interface GGGeolocationDelegate : NSObject<CLLocationManagerDelegate>
{
}
@property (nonatomic, assign) GGGeolocationManager *manager;
@end

class GGGeolocationManager
{
public:
	GGGeolocationManager()
	{
		delegate_ = [[GGGeolocationDelegate alloc] init];
        authStatus_ = [CLLocationManager authorizationStatus];
        locationManager_=nil;
        if ((authStatus_!=kCLAuthorizationStatusDenied)&&(authStatus_!=kCLAuthorizationStatusRestricted))
        {
            locationManager_ = [[CLLocationManager alloc] init];
            locationManager_.delegate = delegate_;
        }
		delegate_.manager = this;
		accuracy_ = 0;
		locationStartCount_ = 0;
		headingStartCount_ = 0;
        
        gid_ = g_NextId();
	}

	~GGGeolocationManager()
	{
        if (locationManager_!=nil)
        {
            locationManager_.delegate = nil;
            [locationManager_ release];
        }
		[delegate_ release];
		
		gevent_RemoveEventsWithGid(gid_);
	}
	
	bool isAvailable()
	{
        if (locationManager_==nil)
            return NO;
        if (([locationManager_ respondsToSelector:@selector(requestWhenInUseAuthorization)])&&(authStatus_==kCLAuthorizationStatusNotDetermined))
            [locationManager_ requestWhenInUseAuthorization];
		BOOL locationServicesEnabledInstancePropertyAvailable = [locationManager_ respondsToSelector:@selector(locationServicesEnabled)]; // iOS 3.x
		BOOL locationServicesEnabledClassPropertyAvailable = [CLLocationManager respondsToSelector:@selector(locationServicesEnabled)]; // iOS 4.x
		
		if (locationServicesEnabledClassPropertyAvailable) 
		{
			return [CLLocationManager locationServicesEnabled];
		} 
		else if (locationServicesEnabledInstancePropertyAvailable) 
		{
			return [(id)locationManager_ locationServicesEnabled];
		} 
		return NO;
	}
	
	bool isHeadingAvailable()
	{
        if (locationManager_==nil)
            return NO;
		BOOL headingInstancePropertyAvailable = [locationManager_ respondsToSelector:@selector(headingAvailable)]; // iOS 3.x
		BOOL headingClassPropertyAvailable = [CLLocationManager respondsToSelector:@selector(headingAvailable)]; // iOS 4.x
		
		if (headingClassPropertyAvailable)
		{
			return [CLLocationManager headingAvailable];
		} 
		else if (headingInstancePropertyAvailable) 
		{
			return [(id)locationManager_ headingAvailable];
		}
		return NO;
	}
	
	void setAccuracy(double desiredAccuracy_num)
	{
        if (locationManager_==nil)
            return;
		accuracy_ = desiredAccuracy_num;
		
		CLLocationAccuracy desiredAccuracy = kCLLocationAccuracyBest;
		if (desiredAccuracy_num < 10)
			desiredAccuracy = kCLLocationAccuracyBest;
		else if (desiredAccuracy_num < 100)
			desiredAccuracy = kCLLocationAccuracyNearestTenMeters;
		else if (desiredAccuracy_num < 1000)
			desiredAccuracy = kCLLocationAccuracyHundredMeters;
		else if (desiredAccuracy_num < 3000)
			desiredAccuracy = kCLLocationAccuracyKilometer;
		else
			desiredAccuracy = kCLLocationAccuracyThreeKilometers;
		
		locationManager_.desiredAccuracy = desiredAccuracy;		
	}
	
	double getAccuracy()
	{
		return accuracy_;
	}
	
	void setThreshold(double threshold)
	{
        if (locationManager_==nil)
            return;
		locationManager_.distanceFilter = threshold;
	}
	
	double getThreshold()
	{
		return locationManager_.distanceFilter;
	}
	
	void startUpdatingLocation()
	{
        if (locationManager_==nil)
            return;
		locationStartCount_++;
		if (locationStartCount_ == 1)
			[locationManager_ startUpdatingLocation];
	}
	
	void stopUpdatingLocation()
	{
        if (locationManager_==nil)
            return;
		if (locationStartCount_ > 0)
		{
			locationStartCount_--;
			if (locationStartCount_ == 0)
				[locationManager_ stopUpdatingLocation];
		}
	}
	
	void startUpdatingHeading()
	{
        if (locationManager_==nil)
            return;
		headingStartCount_++;
		if (headingStartCount_ == 1)
			[locationManager_ startUpdatingHeading];		
	}
	
	void stopUpdatingHeading()
	{
        if (locationManager_==nil)
            return;
		if (headingStartCount_ > 0)
		{
			headingStartCount_--;
			if (headingStartCount_ == 0)
				[locationManager_ stopUpdatingHeading];
		}
	}
	
private:
	CLLocationManager *locationManager_;
    CLAuthorizationStatus authStatus_;
	GGGeolocationDelegate *delegate_;
	double accuracy_;
	int locationStartCount_;
	int headingStartCount_;

public:
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
    
    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GGGeolocationManager*>(udata)->callbackList_.dispatchEvent(type, event);
    }
    
    void enqueueEvent(int type, void *event, int free)
    {
        gevent_EnqueueEvent(gid_, callback_s, type, event, free, this);
    }
    
private:
    gevent_CallbackList callbackList_;
    g_id gid_;
};

@implementation GGGeolocationDelegate

@synthesize manager = manager_;

- (void)locationManager:(CLLocationManager *)manager didUpdateToLocation:(CLLocation *)newLocation fromLocation:(CLLocation *)oldLocation
{
	ggeolocation_LocationUpdateEvent *event = (ggeolocation_LocationUpdateEvent*)malloc(sizeof(ggeolocation_LocationUpdateEvent));		
	event->latitude = newLocation.coordinate.latitude;
	event->longitude = newLocation.coordinate.longitude;
	event->altitude = newLocation.altitude;

	manager_->enqueueEvent(GGEOLOCATION_LOCATION_UPDATE_EVENT, event, 1);
}

- (void)locationManager:(CLLocationManager *)manager didUpdateHeading:(CLHeading *)newHeading
{
	ggeolocation_HeadingUpdateEvent *event = (ggeolocation_HeadingUpdateEvent*)malloc(sizeof(ggeolocation_HeadingUpdateEvent));		
	event->magneticHeading = newHeading.magneticHeading;
	event->trueHeading = newHeading.trueHeading;

	manager_->enqueueEvent(GGEOLOCATION_HEADING_UPDATE_EVENT, event, 1);
}

- (void)locationManager:(CLLocationManager *)manager didFailWithError:(NSError *)error
{
	ggeolocation_ErrorEvent *event = (ggeolocation_ErrorEvent*)malloc(sizeof(ggeolocation_ErrorEvent));		

	manager_->enqueueEvent(GGEOLOCATION_ERROR_EVENT, event, 1);
}

- (BOOL)locationManagerShouldDisplayHeadingCalibration:(CLLocationManager *)manager
{
	return YES;
}

@end


static GGGeolocationManager *s_manager = NULL;

extern "C" {

void ggeolocation_init()
{
    s_manager = new GGGeolocationManager;	
}

void ggeolocation_cleanup()
{
	delete s_manager;
	s_manager = NULL;	
}

int ggeolocation_isAvailable()
{
	return s_manager->isAvailable();	
}
int ggeolocation_isHeadingAvailable()
{
	return s_manager->isHeadingAvailable();
}

void ggeolocation_setAccuracy(double accuracy)
{
	s_manager->setAccuracy(accuracy);
}
double ggeolocation_getAccuracy()
{
	return s_manager->getAccuracy();
}

void ggeolocation_setThreshold(double threshold)
{
	s_manager->setThreshold(threshold);
}
double ggeolocation_getThreshold()
{
	return s_manager->getThreshold();	
}

void ggeolocation_startUpdatingLocation()
{
	s_manager->startUpdatingLocation();
}

void ggeolocation_stopUpdatingLocation()
{
	s_manager->stopUpdatingLocation();
}

void ggeolocation_startUpdatingHeading()
{
	s_manager->startUpdatingHeading();
}

void ggeolocation_stopUpdatingHeading()
{
	s_manager->stopUpdatingHeading();
}

g_id ggeolocation_addCallback(gevent_Callback callback, void *udata)
{
    return s_manager->addCallback(callback, udata);
}

void ggeolocation_removeCallback(gevent_Callback callback, void *udata)
{
    s_manager->removeCallback(callback, udata);
}

void ggeolocation_removeCallbackWithGid(g_id gid)
{
    s_manager->removeCallbackWithGid(gid);    
}

	
}
#endif

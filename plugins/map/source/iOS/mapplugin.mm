#include "mapplugin.h"
#include "AnnotatedMap.h"
#include "gplugin.h"
class MapPluginCPPClass
{
    public:
		MapPluginCPPClass()
		{
			gid_ = g_NextId();
			am = nil;
			
		}

		~MapPluginCPPClass()
		{
			//gevent_RemoveEventsWithGid(gid_);
		}
		
		void initialize(const char *mapplugin)
		{
			if (am == nil)
				am = [[AnnotatedMap alloc] initWithFrame:CGRectMake(0,50,300,250)];
            
            UIViewController *root_view = g_getRootViewController();
            [root_view.view addSubview:am];
		}
		
		void destroy(const char *mapplugin)
		{
			[am deinit];
			am = nil;
		}
		
		void setup(const char *mapplugin, gmapplugin_Parameter *params)
		{
		}

		void mpcpp_setDimensions(const char *mapplugin, int width, int height)
		{
			[am setDimensionsWidth:width Height:height];
		}

		void mpcpp_setPosition(const char *mapplugin, int x, int y)
		{
			[am setPositionX:x Y:y];
		}

		void mpcpp_hide(const char *mapplugin)
		{
			[am hide];
		}

		void mpcpp_show(const char *mapplugin)
		{
			[am show];
		}

		void mpcpp_clear(const char *mapplugin)
		{
			[am show];
		}

		void mpcpp_setCenterCoordinates(const char *mapplugin, double lat, double lon)
		{
			[am setCenterCoordinatesLatitude:lat Longitude:lon];
		}

		void mpcpp_setZoom(const char *mapplugin, int zoom)
		{
			[am setZoom:zoom];
		}

		int mpcpp_mapClicked(const char *mapplugin)
		{
			return [am mapClicked];
		}

		double mpcpp_getMapClickLatitude(const char *mapplugin)
		{
			return [am getMapClickLatitude];
		}

		double mpcpp_getMapClickLongitude(const char *mapplugin)
		{
			return [am getMapClickLongitude];
		}

		void mpcpp_setType(const char *mapplugin, int type)
		{
			[am setType:type];
		}

		double mpcpp_getCenterLatitude(const char *mapplugin)
		{
			return [am getCenterLatitude];
		}

		double mpcpp_getCenterLongitude(const char *mapplugin)
		{
			return [am getCenterLongitude];
		}

		int mpcpp_addMarker(const char *mapplugin, double lat, double lon, const char *title)
		{
            NSString *nstitle = [[NSString alloc] initWithUTF8String:title];
            return [am addMarker:nstitle Latitude:lat Longitude:lon];
		}

		void mpcpp_setMarkerTitle(const char *mapplugin, int idx, const char *title)
		{
            NSString *nstitle = [[NSString alloc] initWithUTF8String:title];
            [am setMarkerTitle:nstitle Index:idx];
		}

		void mpcpp_setMarkerHue(const char *mapplugin, int idx, double hue)
		{
			[am setMarkerHue:hue Index:idx];
		}

		void mpcpp_setMarkerAlpha(const char *mapplugin, int idx, double alpha)
		{
			[am setMarkerAlpha:alpha Index:idx];
		}

		void mpcpp_setMarkerCoordinates(const char *mapplugin, int idx, double lat, double lon)
		{
			[am setMarkerCoordinatesIndex:idx Latitude:lat Longitude:lon];			
		}

		const NSString *mpcpp_getMarkerTitle(const char *mapplugin, int idx)
		{
			return [am getMarkerTitle:idx];
		}

		double mpcpp_getMarkerLatitude(const char *mapplugin, int idx)
		{
			return [am getMarkerLatitude:idx];
		}

		double mpcpp_getMarkerLongitude(const char *mapplugin, int idx)
		{
			return [am getMarkerLongitude:idx];
		}

		int mpcpp_getClickedMarkerIndex(const char *mapplugin)
		{
			return [am getClickedMarkerIndex];
		}
		

	private:
		g_id gid_;
		AnnotatedMap *am;


	};


	static MapPluginCPPClass *s_giab = NULL;

	extern "C" {

	void gmapplugin_init()
	{
		s_giab = new MapPluginCPPClass;
	}

	void gmapplugin_cleanup()
	{
		if(s_giab)
		{
			delete s_giab;
			s_giab = NULL;
		}
	}

	void gmapplugin_initialize(const char *mapplugin)
	{
		if(s_giab)
		{
			s_giab->initialize(mapplugin);
		}
	}

	void gmapplugin_destroy(const char *mapplugin)
	{
		if(s_giab)
		{
			s_giab->destroy(mapplugin);
		}
	}

	void gmapplugin_setup(const char *mapplugin, gmapplugin_Parameter *params)
	{
		if(s_giab)
		{
			s_giab->setup(mapplugin, params);
		}
	}

	void gmapplugin_setDimensions(const char *mapplugin, int width, int height)
	{
		if (s_giab)
		{
			s_giab->mpcpp_setDimensions(mapplugin, width, height);
		}
	}

	void gmapplugin_setPosition(const char *mapplugin, int x, int y){	if (s_giab) { s_giab->mpcpp_setPosition(mapplugin, x, y); } }
	void gmapplugin_hide(const char *mapplugin){	if (s_giab) { s_giab->mpcpp_hide(mapplugin); } }
	void gmapplugin_show(const char *mapplugin){	if (s_giab) { s_giab->mpcpp_show(mapplugin); } }
	void gmapplugin_clear(const char *mapplugin){	if (s_giab) { s_giab->mpcpp_clear(mapplugin); } }
	void gmapplugin_setCenterCoordinates(const char *mapplugin, double lat, double lon){	if (s_giab) { s_giab->mpcpp_setCenterCoordinates(mapplugin, lat, lon); } }
	void gmapplugin_setZoom(const char *mapplugin, int z){	if (s_giab) { s_giab->mpcpp_setZoom(mapplugin, z); } }
	int gmapplugin_mapClicked(const char *mapplugin){	if (s_giab) { return s_giab->mpcpp_mapClicked(mapplugin); } }
	double gmapplugin_getMapClickLatitude(const char *mapplugin){	if (s_giab) { return s_giab->mpcpp_getMapClickLatitude(mapplugin); } }
	double gmapplugin_getMapClickLongitude(const char *mapplugin){	if (s_giab) { s_giab->mpcpp_getMapClickLongitude(mapplugin); } }
	void gmapplugin_setType(const char *mapplugin, int t){	if (s_giab) { s_giab->mpcpp_setType(mapplugin, t); } }
	double gmapplugin_getCenterLatitude(const char *mapplugin){	if (s_giab) { return s_giab->mpcpp_getCenterLatitude(mapplugin); } }
	double gmapplugin_getCenterLongitude(const char *mapplugin){	if (s_giab) { return s_giab->mpcpp_getCenterLongitude(mapplugin); } }
	int gmapplugin_addMarker(const char *mapplugin, double lat, double lon, const char *title){	if (s_giab) { return s_giab->mpcpp_addMarker(mapplugin, lat, lon, title); } }
	void gmapplugin_setMarkerTitle(const char *mapplugin, int idx, const char *title){	if (s_giab) { s_giab->mpcpp_setMarkerTitle(mapplugin, idx, title); } }
	void gmapplugin_setMarkerHue(const char *mapplugin, int idx, double hue){	if (s_giab) { s_giab->mpcpp_setMarkerHue(mapplugin, idx, hue); } }
	void gmapplugin_setMarkerAlpha(const char *mapplugin, int idx, double alpha){	if (s_giab) { s_giab->mpcpp_setMarkerAlpha(mapplugin, idx, alpha); } }
	void gmapplugin_setMarkerCoordinates(const char *mapplugin, int idx, double lat, double lon){	if (s_giab) { s_giab->mpcpp_setMarkerCoordinates(mapplugin, idx, lat, lon); } }
        
	const char *gmapplugin_getMarkerTitle(const char *mapplugin, int idx)
    {
        if (s_giab)
        {
           return [s_giab->mpcpp_getMarkerTitle(mapplugin, idx) UTF8String];
        }
    }
	
    double gmapplugin_getMarkerLatitude(const char *mapplugin, int idx){	if (s_giab) { return s_giab->mpcpp_getMarkerLatitude(mapplugin, idx); } }
	double gmapplugin_getMarkerLongitude(const char *mapplugin, int idx){	if (s_giab) { return s_giab->mpcpp_getMarkerLongitude(mapplugin, idx); } }
	int gmapplugin_getClickedMarkerIndex(const char *mapplugin){	if (s_giab) { return s_giab->mpcpp_getClickedMarkerIndex(mapplugin); } }

}

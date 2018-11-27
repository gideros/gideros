#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <MapKit/MapKit.h>
#import "CustomAnnotation.h"
//#ifndef AnnotatedMap_h
//#define AnnotatedMap_h


#define MAP_TYPE_NORMAL 0
#define MAP_TYPE_SATELLITE 1
#define MAP_TYPE_HYBRID 2
#define MAP_TYPE_TERRAIN 3
#define MAP_TYPE_MAX 4

#define MAP_NO_MARKER_CLICKED -1
#define MAP_INVALID_COORDINATE 1000.0f

#define MAP_MARKER_RED 0.0f
#define MAP_MARKER_ORANGE 22.5f
#define MAP_MARKER_YELLOW 45.0f
#define MAP_MARKER_GREEN 90.0f
#define MAP_MARKER_BLUE 180.0f
#define MAP_MARKER_PURPLE 270.0f
#define MAP_MARKER_HUE_MAX 360.0f

const static int MapTypeToAppleMapType[] =
{
    MKMapTypeStandard,
    MKMapTypeSatellite,
    MKMapTypeHybrid,
    MKMapTypeHybrid // Terrain not supported by Apple - map to hybrid
};


@interface AnnotatedMap:MKMapView
{
    // Instance variables (private)
    int mMapX;
    int mMapY;
    int mMapWidth;
    int mMapHeight;
    int mNumMarkers;
    bool mMapClicked;
    int mMarkerClicked;
    float mClickLatitude;
    float mClickLongitude;
    NSMutableArray *mMarkerArray;
    UIView *mParentView;
}

// Properties (public)
//@property (nonatomic) IBOutlet MKMapView *map;

// Method prototypes:
-(void) setDimensionsWidth:(int)w Height:(int)h;
-(void) setPositionX:(int)x Y:(int)y;
-(void) setDefaults;
-(void) hide;
-(void) show;
-(void) setCenterCoordinatesLatitude:(float)latitude Longitude:(float)longitude;
-(void) setZoom:(float) zoom;
-(bool) mapClicked;
-(float) getMapClickLatitude;
-(float) getMapClickLongitude;
-(void) setType:(int) type;
-(void) clear;
-(float) getCenterLatitude;
-(float) getCenterLongitude;
-(int) addMarker:(NSString *)title Latitude:(float)latitude Longitude: (float)longitude;
-(void) setMarkerColorIndex:(int)index Red:(float)red Green:(float)green Blue:(float)blue Alpha:(float)alpha;
-(void) setMarkerTitle:(NSString *)title Index:(int)index;
-(void) setMarkerHue:(float) hue Index:(int) index;
-(void) setMarkerAlpha:(float) alpha Index:(int) index;
-(void) setMarkerCoordinatesIndex:(int)index Latitude:(float)latitude Longitude:(float) longitude;
-(NSString *) getMarkerTitle:(int) index;
-(float) getMarkerLatitude:(int) index;
-(float) getMarkerLongitude:(int) index;
-(int) getClickedMarkerIndex;


@end


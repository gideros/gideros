#import "AnnotatedMap.h"
@implementation AnnotatedMap:MKMapView

-(instancetype)init
{
    self = [super init];
    [self setDefaults];
}

-(void) setDefaults
{
    mMapX = 0;
    mMapY = 0;
    mMapWidth = 800;
    mMapHeight = 400;
    mNumMarkers = 0;
    mMarkerArray = [[NSMutableArray alloc] init];
    mParentView = nil;
    
    mMapClicked = false;
    mMarkerClicked = MAP_NO_MARKER_CLICKED;
    mClickLatitude = MAP_INVALID_COORDINATE;
    mClickLongitude = MAP_INVALID_COORDINATE;
    
    // Add a gesture recognizer to the map.
    UILongPressGestureRecognizer *lPGR =
    [[UILongPressGestureRecognizer alloc] initWithTarget:self action:@selector(mapLongPress:)];
   
    // Detect 1 second taps:
    // If we use a shorter duration then other gestures like doubletaps to zoom in
    // or the beginning of a swipe to pan get consumed by the gesture recognizer.
    // It may be possible to detect the taps
    // without consuming them.
    lPGR.minimumPressDuration = 1.0;
    [self addGestureRecognizer:lPGR];
 }


-(instancetype)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
    {
        self.delegate = self;
        [self setDefaults];
    }
    return self;
}

-(void) setDimensionsWidth:(int)w Height:(int)h
{
    mMapWidth = w;
    mMapHeight = h;
    self.frame = CGRectMake(mMapX,mMapY, mMapWidth, mMapHeight);
}

-(void) setPositionX:(int)x Y:(int)y
{
 
    mMapX = x;
    mMapY = y;
    self.frame = CGRectMake(mMapX, mMapY, mMapWidth, mMapHeight);
}

-(MKAnnotationView *) mapView:(MKMapView *) mapView viewForAnnotation:(nonnull id<MKAnnotation>)annotation
{
    
    MKAnnotationView *annotationView = [[MKAnnotationView alloc] initWithAnnotation: annotation reuseIdentifier:@"AnnotationIdentifier"];
    
    
    UIImage *image = [[UIImage imageNamed: @"marker20x30"] imageWithRenderingMode: UIImageRenderingModeAlwaysTemplate];
    
    
    CustomAnnotation *ca = (CustomAnnotation *) annotation;
    
    UIGraphicsBeginImageContextWithOptions(image.size, NO, image.scale);
    CGContextRef context = UIGraphicsGetCurrentContext();
    CGContextTranslateCTM(context, 0, image.size.height);
    CGContextScaleCTM(context, 1.0, -1.0);
    CGContextSetBlendMode(context, kCGBlendModeNormal);
    CGRect rect = CGRectMake(0,0, image.size.width, image.size.height);
    CGContextClipToMask(context, rect, image.CGImage);
    UIColor *col = [UIColor colorWithRed: ca.red green:ca.green blue:ca.blue alpha:ca.alpha];
    [col setFill];
    CGContextFillRect(context, rect);
    UIImage *newImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    annotationView.image = newImage;
    annotationView.canShowCallout = YES;
    [annotationView setCenterOffset: CGPointMake(0,-15)];

    return annotationView;
}

- (int) addMarker:(NSString *)title Latitude:(float)latitude Longitude: (float)longitude
{
    
    mNumMarkers++;
    mMarkerArray[mNumMarkers-1] = [[CustomAnnotation alloc] init];
    
    CLLocationCoordinate2D coords;
    coords.latitude = latitude;
    coords.longitude = longitude;
    CustomAnnotation *point = [[CustomAnnotation alloc] init];
    [point setCoordinate: coords];
    point.red = 1.0;
    point.green = 0.5;
    point.blue = 0.5;
    point.alpha = 1.0;
    point.index = mNumMarkers - 1;
    
    point.title = title;
    [self addAnnotation:point];
    [mMarkerArray insertObject:point atIndex:point.index];

    return point.index;
}



-(void) setMarkerColorIndex:(int)index Red:(float)red Green:(float)green Blue:(float)blue Alpha:(float)alpha
{
    CustomAnnotation *ca = [mMarkerArray objectAtIndex:index];
    if (ca)
    {
        ca.red = red;
        ca.green = green;
        ca.blue = blue;
        ca.alpha = alpha;
    }
    
}

-(void) hide
{
    mParentView = [self superview];
    [self removeFromSuperview];
}

-(void) show
{
    // must add this object to a view and call hide before calling show
    if (mParentView)
    {
        [mParentView addSubview:self];
    }
}

-(void) setMyLocationEnabled:(bool) enabled
{
    self.showsUserLocation = enabled;
}

-(void) setCenterCoordinatesLatitude:(float)latitude Longitude:(float)longitude
{
    CLLocationCoordinate2D coords;
    coords.latitude = latitude;
    coords.longitude = longitude;
    [self setCenterCoordinate:coords];
}

-(bool) mapClicked
{
    if (mMapClicked)
    {
        mMapClicked = false;
        return true;
    }
    return false;
}

-(void) setZoom:(float) zoom
{
    // Map a zoom level of 1 (show a very wide area) to 20 (closeup of very small area,
    // to something comparable to the zoom levels used by Google Maps for a consistent
    // interface
    int scale_array[] =
    {
        591658,
        295829,
        147914,
        73957,
        36979,
        18489,
        9245,
        4622,
        2311,
        1156,
        578,
        289,
        144,
        72,
        36,
        18,
        9,
        5,
        2,
        1
    };
    
    const int max_zoom = 20;
 
    int zoom_index = (int) zoom;
    zoom_index = MIN(zoom_index, max_zoom - 1);
    zoom_index = MAX(zoom_index, 0);
    
    float scale = scale_array[zoom_index];
    float interpolation = zoom - floor(zoom);
    if (1 > interpolation && interpolation > 0 && zoom_index < (max_zoom - 2))
    {
        scale += (float) (scale_array[zoom_index + 1] - scale_array[zoom_index]) * interpolation;
    }
    float field_of_view = scale * 20;
    
    MKCoordinateRegion viewRegion = MKCoordinateRegionMakeWithDistance(self.centerCoordinate, field_of_view, field_of_view);
    MKCoordinateRegion adjustedRegion = [self regionThatFits:viewRegion];
    [self setRegion:adjustedRegion animated:NO];
    
}

-(void) mapLongPress:(UIGestureRecognizer*)gesture
{
    if (gesture.state != UIGestureRecognizerStateBegan)
        return;
    
    // Note the location on that map that was clicked:
    
    CGPoint point = [gesture locationInView:self];
    CLLocationCoordinate2D coords = [self convertPoint:point toCoordinateFromView:self];
    
    mClickLatitude = coords.latitude;
    mClickLongitude = coords.longitude;
    mMapClicked = true;
}

-(float) getMapClickLatitude
{
    return mClickLatitude;
}

-(float) getMapClickLongitude
{
    return mClickLongitude;
}

-(void) setType:(int)type
{
    if (type >= 0 && type < MAP_TYPE_MAX)
    {
        self.mapType = MapTypeToAppleMapType[type];
    }
}

-(void) clear
{
    [self removeAnnotations:self.annotations];
    mNumMarkers = 0;
    [mMarkerArray removeAllObjects];
    
}

-(float) getCenterLatitude
{
    return self.centerCoordinate.latitude;
}

-(float) getCenterLongitude
{
    return self.centerCoordinate.longitude;
    
}


-(void) setMarkerTitle:(NSString *)title Index:(int)index
{
    CustomAnnotation *ca = [mMarkerArray objectAtIndex:index];
    if (ca)
    {
        ca.title = title;
    }
    
}


-(void) setMarkerHue:(float) hue Index:(int) index
{
    // Convert Google hue to RGB values to tint a marker
    
    CustomAnnotation *ca = [mMarkerArray objectAtIndex:index];
    if (ca)
    {
        // Hue ranges from 0 (red) through 360 (back to red) passing through orange, yellow,
        // green, blue and purple
        float red, green, blue;
        float interpolate;
        float low_red, low_green, low_blue;
        float high_red, high_green, high_blue;
        
        // RGB mix for each color
        
        const float red_red = 1.0;
        const float red_green = 0.0;
        const float red_blue = 0.0;
        
        const float orange_red = 1.0;
        const float orange_green = 0.5;
        const float orange_blue = 0.0;
        
        const float yellow_red = 1.0;
        const float yellow_green = 1.0;
        const float yellow_blue = 0.0;
        
        const float green_red = 0.0;
        const float green_green = 1.0;
        const float green_blue = 0.0;
        
        const float blue_red = 0.0;
        const float blue_green = 0.0;
        const float blue_blue = 1.0;
        
        const float purple_red = 1.0;
        const float purple_green = 0.0;
        const float purple_blue = 1.0;
        
        const float max_red = 1.0;
        const float max_green = 0.0;
        const float max_blue = 0.0;
        
        float hue_low, hue_high;
        
        if (hue < MAP_MARKER_ORANGE)
        {
            low_red = red_red;
            low_green = red_green;
            low_blue = red_blue;
            high_red = orange_red;
            high_green = orange_green;
            high_blue = orange_blue;
            hue_low = MAP_MARKER_RED;
            hue_high = MAP_MARKER_ORANGE;
            
        }
        else if (hue < MAP_MARKER_YELLOW)
        {
            low_red = orange_red;
            low_green = orange_green;
            low_blue = orange_blue;
            high_red = yellow_red;
            high_green = yellow_green;
            high_blue = yellow_blue;
            hue_low = MAP_MARKER_ORANGE;
            hue_high = MAP_MARKER_YELLOW;
            
        }
        else if (hue < MAP_MARKER_GREEN)
        {
            low_red = yellow_red;
            low_green = yellow_green;
            low_blue = yellow_blue;
            high_red = green_red;
            high_green = green_green;
            high_blue = green_blue;
            
            hue_low = MAP_MARKER_YELLOW;
            hue_high = MAP_MARKER_GREEN;
        }
        else if (hue < MAP_MARKER_BLUE)
        {
            
            low_red = green_red;
            low_green = green_green;
            low_blue = green_blue;
            high_red = blue_red;
            high_green = blue_green;
            high_blue = blue_blue;
            
            hue_low = MAP_MARKER_GREEN;
            hue_high = MAP_MARKER_BLUE;
        }
        else if (hue < MAP_MARKER_PURPLE)
        {
            
            low_red = blue_red;
            low_green = blue_green;
            low_blue = blue_blue;
            high_red = purple_red;
            high_green = purple_green;
            high_blue = purple_blue;
            hue_low = MAP_MARKER_BLUE;
            hue_high = MAP_MARKER_PURPLE;
        }
        else
        {
            low_red = purple_red;
            low_green = purple_green;
            low_blue = purple_blue;
            high_red = max_red;
            high_green = max_green;
            high_blue = max_blue;
            
            hue_low = MAP_MARKER_PURPLE;
            hue_high = MAP_MARKER_HUE_MAX;
        }
        interpolate = (hue - hue_low) / (hue_high - hue_low);

        ca.red = low_red + (high_red - low_red) * interpolate;
        ca.green = low_green + (high_green - low_green) * interpolate;
        ca.blue = low_blue + (high_blue - low_blue) * interpolate;
    }
}

-(void) setMarkerAlpha:(float) alpha Index:(int) index
{
    CustomAnnotation *ca = [mMarkerArray objectAtIndex:index];
    if (ca)
    {
        ca.alpha = alpha;
    }
}

-(void) setMarkerCoordinatesIndex:(int)index Latitude:(float)latitude Longitude:(float) longitude
{
    CustomAnnotation *ca = [mMarkerArray objectAtIndex:index];
    if (ca)
    {
        CLLocationCoordinate2D coords;
        coords.latitude = latitude;
        coords.longitude = longitude;
        [ca setCoordinate: coords];
    }
}

-(NSString *) getMarkerTitle:(int)index
{
    CustomAnnotation *ca = [mMarkerArray objectAtIndex:index];
    if (ca)
    {
        return (ca.title);
    }
    return nil;
}

 -(float) getMarkerLatitude:(int)index
{
    CustomAnnotation *ca = [mMarkerArray objectAtIndex:index];
    if (ca)
    {
        return ca.coordinate.latitude;
    }
    return MAP_INVALID_COORDINATE;
}

 -(float) getMarkerLongitude:(int)index
{
    CustomAnnotation *ca = [mMarkerArray objectAtIndex:index];
    if (ca)
    {
        return ca.coordinate.longitude;
    }
    return MAP_INVALID_COORDINATE;
    
}

-(void)mapView:(MKMapView *)mapView didSelectAnnotationView:(MKAnnotationView *)view
{
    CustomAnnotation *ca = (CustomAnnotation *) view.annotation;
    mMarkerClicked = ca.index;
}

 -(int) getClickedMarkerIndex
{
    int return_val = mMarkerClicked;
    mMarkerClicked = MAP_NO_MARKER_CLICKED;
    return return_val;
    
}




@end

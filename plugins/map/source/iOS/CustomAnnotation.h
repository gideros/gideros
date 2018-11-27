

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import <MapKit/MapKit.h>

@interface CustomAnnotation: NSObject<MKAnnotation>
@property (nonatomic, assign) CLLocationCoordinate2D coordinate;
@property (nonatomic, copy) NSString *title;
@property (nonatomic, copy) NSString *subtitle;
@property float red;
@property float green;
@property float blue;
@property float alpha;
@property int index;
-(id) initWithLocation:(CLLocationCoordinate2D) coord;
@end

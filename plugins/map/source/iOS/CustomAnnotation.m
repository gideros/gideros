#import "CustomAnnotation.h"
@implementation CustomAnnotation

@synthesize coordinate;
@synthesize title;
@synthesize subtitle;
@synthesize red;
@synthesize green;
@synthesize blue;
@synthesize alpha;
@synthesize index;

-(id) initWithLocation:(CLLocationCoordinate2D)coord{
    self = [super init];
    
    if (self)
    {
        coordinate = coord;
    }
    return self;
}

@end

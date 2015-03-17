//
//  AdsProtocol.h
//  Ads
//
//  Created by Arturs Sosins on 6/25/13.
//  Copyright (c) 2013 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>

@protocol ControllerProtocol <NSObject>

-(id)init;
-(void)destroy;
-(NSString*)getControllerName:(NSString*)nid;
@end

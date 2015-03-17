//
//  GControllerDefault.h
//  Test
//
//  Created by Arturs Sosins on 1/9/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "ControllerProtocol.h"
#import "iCadeReaderView.h"
#import "GController.h"

@interface GControllerIcade : NSObject <ControllerProtocol, iCadeEventDelegate>
@property (nonatomic) NSInteger lastId;
@property (nonatomic, retain) GController* controller;
@end

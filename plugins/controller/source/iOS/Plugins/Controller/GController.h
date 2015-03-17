//
//  GController.h
//  Test
//
//  Created by Arturs Sosins on 1/9/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface GControllerStick : NSObject

@property (nonatomic)int playerId;
@property (nonatomic)float lastX;
@property (nonatomic)float lastY;
@property (nonatomic)BOOL isLeft;

-(id)init:(int)nid isLeft:(BOOL) left;
-(void)handleAxis:(float)x with:(float)y;
@end

@interface GControllerTrigger : NSObject

@property (nonatomic)int playerId;
@property (nonatomic)float lastValue;
@property (nonatomic)BOOL isLeft;

-(id)init:(int)nid isLeft:(BOOL) left;
-(void)handleTrigger:(float)value;
@end

@interface GControllerButton : NSObject

@property (nonatomic)int playerId;
@property (nonatomic)int keyCode;
@property (nonatomic)BOOL isDown;

-(id)init:(int)nid withCode:(int) code;
-(void)handlePositiveButton:(float)value;
-(void)handleNegativeButton:(float)value;
@end

@interface GController : NSObject

@property (nonatomic, retain)NSNumber* playerId;
@property (nonatomic, retain)GControllerStick* rightStick;
@property (nonatomic, retain)GControllerStick* leftStick;
@property (nonatomic, retain)GControllerTrigger* rightTrigger;
@property (nonatomic, retain)GControllerTrigger* leftTrigger;
@property (nonatomic, retain)GControllerButton* R2Button;
@property (nonatomic, retain)GControllerButton* L2Button;
@property (nonatomic, retain)GControllerButton* DPadUp;
@property (nonatomic, retain)GControllerButton* DPadDown;
@property (nonatomic, retain)GControllerButton* DPadRight;
@property (nonatomic, retain)GControllerButton* DPadLeft;

-(id)init:(NSNumber*)nid;
-(void)destroy;
-(NSNumber*)getPlayerId;
-(void)onKeyDown:(NSInteger)keyCode;
-(void)onKeyUp:(NSInteger)keyCode;
-(void)handleRightStick:(float)x with:(float)y;
-(void)handleLeftStick:(float)x with:(float)y;
-(void)handleRightTrigger:(float)value;
-(void)handleLeftTrigger:(float)value;
-(void)handleL2Button:(float) value;
-(void)handleR2Button:(float) value;
-(void)handleDPadUpButton:(float)value;
-(void)handleDPadDownButton:(float) value;
-(void)handleDPadLeftButton:(float) value;
-(void)handleDPadRightButton:(float) value;
@end

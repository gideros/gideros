//
//  GController.m
//  Test
//
//  Created by Arturs Sosins on 1/9/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//
#include "controller.h"
#import "GController.h"

static float STICK_DEADZONE = 0.25f;

@implementation GController

-(id)init:(NSNumber*)nid{
    self.playerId = nid;
    
    self.rightStick = [[GControllerStick alloc] init:[self.playerId intValue] isLeft:NO];
    self.leftStick = [[GControllerStick alloc] init:[self.playerId intValue] isLeft:YES];
    
    self.rightTrigger = [[GControllerTrigger alloc] init:[self.playerId intValue] isLeft:NO];
    self.leftTrigger = [[GControllerTrigger alloc] init:[self.playerId intValue] isLeft:YES];
    
    self.R2Button = [[GControllerButton alloc] init:[self.playerId intValue] withCode:BUTTON_R2];
    self.L2Button = [[GControllerButton alloc] init:[self.playerId intValue] withCode:BUTTON_R2];
    
    self.DPadDown = [[GControllerButton alloc] init:[self.playerId intValue] withCode:DPAD_DOWN];
    self.DPadUp = [[GControllerButton alloc] init:[self.playerId intValue] withCode:DPAD_UP];
    self.DPadRight = [[GControllerButton alloc] init:[self.playerId intValue] withCode:DPAD_RIGHT];
    self.DPadLeft = [[GControllerButton alloc] init:[self.playerId intValue] withCode:DPAD_LEFT];
    
    ghid_onConnected([nid intValue]);
    return self;
}

-(void)destroy{
    ghid_onDisconnected([self.playerId intValue]);
    
    [self.rightStick release];
    [self.leftStick release];
    [self.rightTrigger release];
    [self.leftTrigger release];
    [self.DPadDown release];
    [self.DPadUp release];
    [self.DPadRight release];
    [self.DPadLeft release];
    
    self.rightStick = nil;
    self.leftStick = nil;
    self.rightTrigger = nil;
    self.leftTrigger = nil;
    self.DPadDown = nil;
    self.DPadUp = nil;
    self.DPadRight = nil;
    self.DPadLeft = nil;
}

-(NSNumber*)getPlayerId{
    return self.playerId;
}

-(void)onKeyDown:(NSInteger)keyCode{
    ghid_onKeyDownEvent(keyCode, [self.playerId intValue]);
}

-(void)onKeyUp:(NSInteger)keyCode{
    ghid_onKeyUpEvent(keyCode, [self.playerId intValue]);
}

-(void)handleRightStick:(float)x with:(float)y{
    [self.rightStick handleAxis:x with:y];
}

-(void)handleLeftStick:(float)x with:(float)y{
    [self.leftStick handleAxis:x with:y];
}

-(void)handleRightTrigger:(float)value{
    [self.rightTrigger handleTrigger:value];
}

-(void)handleLeftTrigger:(float)value{
    [self.leftTrigger handleTrigger:value];
}

-(void)handleL2Button:(float) value{
    [self.L2Button handlePositiveButton:value];
}

-(void)handleR2Button:(float) value{
    [self.R2Button handlePositiveButton:value];
}

-(void)handleDPadUpButton:(float)value{
    [self.DPadUp handleNegativeButton:value];
}

-(void)handleDPadDownButton:(float) value{
    [self.DPadDown handlePositiveButton:value];
}

-(void) handleDPadLeftButton:(float) value{
    [self.DPadLeft handleNegativeButton:value];
}

-(void) handleDPadRightButton:(float) value{
    [self.DPadRight handlePositiveButton:value];
}

@end

@implementation GControllerStick

-(id)init:(int)nid isLeft:(BOOL) left{
    self.playerId = nid;
    self.isLeft = left;
    self.lastX = 0;
    self.lastY = 0;
    return self;
}

-(void)handleAxis:(float)x with:(float)y{
    if(x*x + y*y <= STICK_DEADZONE * STICK_DEADZONE)
    {
        x = 0;
        y = 0;
    }
    
    if(self.lastX != x || self.lastY != y)
    {
        self.lastX = x;
        self.lastY = y;
        double strength = sqrt(x*x + y*y);
        double angle = acos(x/strength);
        if(y>0)
        {
            angle= -angle+2*M_PI;
        }
        angle = -angle+2*M_PI;
        if(self.isLeft)
        {
            ghid_onLeftJoystick(x, y, angle, strength, self.playerId);
        }
        else
        {
            ghid_onRightJoystick(x, y, angle, strength, self.playerId);
        }
    }
}
@end

@implementation GControllerTrigger

-(id)init:(int)nid isLeft:(BOOL) left{
    self.playerId = nid;
    self.isLeft = left;
    self.lastValue = 0;
    return self;
}

-(void)handleTrigger:(float)value{
    if(value < STICK_DEADZONE){
        value = 0;
    }
    
    if(self.lastValue != value){
        self.lastValue = value;
        if(self.isLeft)
            ghid_onLeftTrigger(value, self.playerId);
        else
            ghid_onRightTrigger(value, self.playerId);
    }
}
@end

@implementation GControllerButton

-(id)init:(int)nid withCode:(int) code{
    self.playerId = nid;
    self.keyCode = code;
    self.isDown = NO;
    return self;
}

-(void)handlePositiveButton:(float)value{
    if(!self.isDown && value > 0.5)
    {
        self.isDown = true;
        ghid_onKeyDownEvent(self.keyCode, self.playerId);
    }
    else if(self.isDown && value < 0.5)
    {
        self.isDown = false;
        ghid_onKeyUpEvent(self.keyCode, self.playerId);
    }
}

-(void)handleNegativeButton:(float)value{
    if(!self.isDown && value < -0.5)
    {
        self.isDown = true;
        ghid_onKeyDownEvent(self.keyCode, self.playerId);
    }
    else if(self.isDown && value > -0.5)
    {
        self.isDown = false;
        ghid_onKeyUpEvent(self.keyCode, self.playerId);
    }
}
@end


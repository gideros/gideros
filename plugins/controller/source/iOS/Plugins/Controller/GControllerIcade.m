//
//  GControllerDefault.m
//  Test
//
//  Created by Arturs Sosins on 1/9/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//
#include "controller.h"
#include "gideros.h"
#import "GControllerIcade.h"
#import "GControllerManager.h"

@implementation GControllerIcade

-(id)init{
    self.controller = nil;
    iCadeReaderView *icrv = [[iCadeReaderView alloc] initWithFrame:CGRectZero];
    UIViewController *root = g_getRootViewController();
    [root.view addSubview:icrv];
    icrv.active = YES;
    icrv.delegate = self;
    return self;
}

-(void)destroy{

}

-(void)addDevice{
    self.controller = [GControllerManager getController:[self getIdentifier] withType:@"GControllerIcade"];
}

-(NSString*)getControllerName:(NSString*)nid{
    return @"Bluetooth Controller";
}

-(NSString*)getIdentifier{
    NSString* nid = @"1";
    return [@"Icade_" stringByAppendingString:nid];
}

-(void)buttonDown:(iCadeState)button {
    if(self.controller == nil)
        [self addDevice];
    
    if (button == iCadeButtonA) {
        [self.controller onKeyDown:BUTTON_X];
    } else if (button == iCadeButtonB) {
        [self.controller onKeyDown:BUTTON_A];
    } else if (button == iCadeButtonC) {
        [self.controller onKeyDown:BUTTON_Y];
    } else if (button == iCadeButtonD) {
        [self.controller onKeyDown:BUTTON_B];
    } else if (button == iCadeButtonE) {
        [self.controller onKeyDown:BUTTON_L1];
    } else if (button == iCadeButtonF) {
        [self.controller onKeyDown:BUTTON_R1];
    } else if (button == iCadeButtonG) {
        [self.controller onKeyDown:BUTTON_MENU];
    } else if (button == iCadeButtonH) {
        [self.controller onKeyDown:BUTTON_BACK];
    }
}

-(void)buttonUp:(iCadeState)button {
    if(self.controller == nil)
        [self addDevice];
    if (button == iCadeButtonA) {
        [self.controller onKeyUp:BUTTON_X];
    } else if (button == iCadeButtonB) {
        [self.controller onKeyUp:BUTTON_A];
    } else if (button == iCadeButtonC) {
        [self.controller onKeyUp:BUTTON_Y];
    } else if (button == iCadeButtonD) {
        [self.controller onKeyUp:BUTTON_B];
    } else if (button == iCadeButtonE) {
        [self.controller onKeyUp:BUTTON_L1];
    } else if (button == iCadeButtonF) {
        [self.controller onKeyUp:BUTTON_R1];
    } else if (button == iCadeButtonG) {
        [self.controller onKeyUp:BUTTON_MENU];
    } else if (button == iCadeButtonH) {
        [self.controller onKeyUp:BUTTON_BACK];
    }
}

-(void)stateChanged:(iCadeState)button{
    if (button == iCadeJoystickLeft) {
        [self.controller handleDPadLeftButton:-1];
        [self.controller handleDPadRightButton:0];
        [self.controller handleLeftStick:-1 with:0];
    } else if (button == iCadeJoystickRight) {
        [self.controller handleDPadLeftButton:0];
        [self.controller handleDPadRightButton:1];
        [self.controller handleLeftStick:1 with:0];
    } else if (button == iCadeJoystickUp) {
        [self.controller handleDPadUpButton:-1];
        [self.controller handleDPadDownButton:0];
        [self.controller handleLeftStick:0 with:-1];
    } else if (button == iCadeJoystickDown) {
        [self.controller handleDPadUpButton:0];
        [self.controller handleDPadDownButton:1];
        [self.controller handleLeftStick:0 with:1];
    } else if (button == iCadeJoystickUpLeft) {
        [self.controller handleDPadUpButton:-1];
        [self.controller handleDPadDownButton:0];
        [self.controller handleDPadLeftButton:-1];
        [self.controller handleDPadRightButton:0];
        [self.controller handleLeftStick:-1 with:-1];
    } else if (button == iCadeJoystickUpRight) {
        [self.controller handleDPadUpButton:-1];
        [self.controller handleDPadDownButton:0];
        [self.controller handleDPadLeftButton:0];
        [self.controller handleDPadRightButton:1];
        [self.controller handleLeftStick:1 with:-1];
    } else if (button == iCadeJoystickDownLeft) {
        [self.controller handleDPadUpButton:0];
        [self.controller handleDPadDownButton:1];
        [self.controller handleDPadLeftButton:-1];
        [self.controller handleDPadRightButton:0];
        [self.controller handleLeftStick:-1 with:1];
    } else if (button == iCadeJoystickDownRight) {
        [self.controller handleDPadUpButton:0];
        [self.controller handleDPadDownButton:1];
        [self.controller handleDPadLeftButton:0];
        [self.controller handleDPadRightButton:1];
        [self.controller handleLeftStick:1 with:1];
    } else if (button == iCadeJoystickNone) {
        [self.controller handleDPadUpButton:0];
        [self.controller handleDPadDownButton:0];
        [self.controller handleDPadLeftButton:0];
        [self.controller handleDPadRightButton:0];
        [self.controller handleLeftStick:0 with:0];
    }
}

@end

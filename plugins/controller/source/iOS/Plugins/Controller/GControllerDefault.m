//
//  GControllerDefault.m
//  Test
//
//  Created by Arturs Sosins on 1/9/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//
#include "controller.h"
#import "GControllerDefault.h"
#import "GControllerManager.h"
#import "GController.h"
#import <GameController/GCController.h>

@implementation GControllerDefault

-(id)init{
    self.lastId = 0;
    self.controllers = [NSMutableDictionary dictionary];
    if([GCController class] != nil)
    {
        [GCController startWirelessControllerDiscoveryWithCompletionHandler:^() {
                NSLog(@"Discovery ended");
        }];
        NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
        [center addObserver:self selector:@selector(onConnect:) name:GCControllerDidConnectNotification object:nil];
        [center addObserver:self selector:@selector(onDisconnect:) name:GCControllerDidDisconnectNotification object:nil];
    
        //check already connected controllers
    
        for(GCController* c in GCController.controllers){
            [self addDevice:c];
        }
    }
    
    return self;
}

-(void)addDevice:(GCController*)c{
    c.playerIndex = self.lastId;
    self.lastId++;
    NSString* nid = [NSString stringWithFormat:@"%d",c.playerIndex];
    [self.controllers setObject:c forKey:nid];
    if (c.extendedGamepad) {
        c.extendedGamepad.valueChangedHandler =  ^(GCExtendedGamepad *gamepad, GCControllerElement *element)
        {
            GController* gc = [GControllerManager getController:[self getIdentifier:c.playerIndex] withType:@"GControllerDefault"];
            if (gamepad.rightThumbstick == element)
                [gc handleRightStick:gamepad.rightThumbstick.xAxis.value with:gamepad.rightThumbstick.yAxis.value];
            else if (gamepad.leftThumbstick == element)
                [gc handleLeftStick:gamepad.leftThumbstick.xAxis.value with:gamepad.leftThumbstick.yAxis.value];
            else if (gamepad.rightTrigger == element)
            {
                [gc handleRightTrigger:gamepad.rightTrigger.value];
                [gc handleR2Button:gamepad.rightTrigger.value];
            }
            else if (gamepad.leftTrigger == element)
            {
                [gc handleLeftTrigger:gamepad.leftTrigger.value];
                [gc handleL2Button:gamepad.leftTrigger.value];
            }
            else if (gamepad.rightShoulder == element)
                if(gamepad.rightShoulder.pressed)
                    [gc onKeyDown:BUTTON_R1];
                else
                    [gc onKeyUp:BUTTON_R1];
            else if (gamepad.leftShoulder == element)
                if(gamepad.leftShoulder.pressed)
                    [gc onKeyDown:BUTTON_L1];
                else
                    [gc onKeyUp:BUTTON_L1];
            else if (gamepad.buttonA == element)
                if(gamepad.buttonA.pressed)
                    [gc onKeyDown:BUTTON_A];
                else
                    [gc onKeyUp:BUTTON_A];
            else if (gamepad.buttonB == element)
                if(gamepad.buttonB.pressed)
                    [gc onKeyDown:BUTTON_B];
                else
                    [gc onKeyUp:BUTTON_B];
            else if (gamepad.buttonX == element)
                if(gamepad.buttonX.pressed)
                    [gc onKeyDown:BUTTON_X];
                else
                    [gc onKeyUp:BUTTON_X];
            else if (gamepad.buttonY == element)
                if(gamepad.buttonY.pressed)
                    [gc onKeyDown:BUTTON_Y];
                else
                    [gc onKeyUp:BUTTON_Y];
            else if(gamepad.dpad == element)
            {
                [gc handleDPadUpButton:gamepad.dpad.yAxis.value];
                [gc handleDPadDownButton:gamepad.dpad.yAxis.value];
                [gc handleDPadLeftButton:gamepad.dpad.xAxis.value];
                [gc handleDPadRightButton:gamepad.dpad.xAxis.value];
            }
        };
    }
    else if(c.gamepad){
        c.gamepad.valueChangedHandler =  ^(GCGamepad *gamepad, GCControllerElement *element)
        {
            GController* gc = [GControllerManager getController:[self getIdentifier:c.playerIndex] withType:@"GControllerDefault"];
            if (gamepad.rightShoulder == element)
                if(gamepad.rightShoulder.pressed)
                    [gc onKeyDown:BUTTON_R1];
                else
                    [gc onKeyUp:BUTTON_R1];
            else if (gamepad.leftShoulder == element)
                if(gamepad.leftShoulder.pressed)
                    [gc onKeyDown:BUTTON_L1];
                else
                    [gc onKeyUp:BUTTON_L1];
            else if (gamepad.buttonA == element)
                if(gamepad.buttonA.pressed)
                    [gc onKeyDown:BUTTON_A];
                else
                    [gc onKeyUp:BUTTON_A];
            else if (gamepad.buttonB == element)
                if(gamepad.buttonB.pressed)
                    [gc onKeyDown:BUTTON_B];
                else
                    [gc onKeyUp:BUTTON_B];
            else if (gamepad.buttonX == element)
                if(gamepad.buttonX.pressed)
                    [gc onKeyDown:BUTTON_X];
                else
                    [gc onKeyUp:BUTTON_X];
            else if (gamepad.buttonY == element)
                if(gamepad.buttonY.pressed)
                    [gc onKeyDown:BUTTON_Y];
                else
                    [gc onKeyUp:BUTTON_Y];
            else if(gamepad.dpad == element)
            {
                [gc handleDPadUpButton:gamepad.dpad.yAxis.value];
                [gc handleDPadDownButton:gamepad.dpad.yAxis.value];
                [gc handleDPadLeftButton:gamepad.dpad.xAxis.value];
                [gc handleDPadRightButton:gamepad.dpad.xAxis.value];
                [gc handleLeftStick:gamepad.dpad.xAxis.value with:gamepad.dpad.yAxis.value];
            }
        };
    }
}

-(void)remDevice:(GCController*)c{
    [self.controllers removeObjectForKey:[NSString stringWithFormat:@"%d",c.playerIndex]];
    [GControllerManager remDevice:[self getIdentifier:c.playerIndex]];
}

-(void)destroy{
    [GCController stopWirelessControllerDiscovery];
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self];
}

-(NSString*)getControllerName:(NSString*)nid{
    GCController* c = [self.controllers objectForKey:nid];
    if(c)
    {
        return c.vendorName;
    }
    return @"MFI Controller";
}

-(void)onConnect:(NSNotification*)note{
    [self addDevice:note.object];
}

-(void)onDisconnect:(NSNotification*)note{
    [self remDevice:note.object];
}

-(NSString*)getIdentifier:(NSInteger)n{
    NSString* nid = [NSString stringWithFormat:@"%d",n];
    return [@"MFI_" stringByAppendingString:nid];
}

@end

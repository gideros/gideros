//
//  GMediaClass.h
//  Test
//
//  Created by Arturs Sosins on 1/17/14.
//  Copyright (c) 2014 Gideros Mobile. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import "AVFoundation/AVFoundation.h"

@interface GMediaView : UIView
@property(nonatomic, retain)AVPlayerLayer *Alayer;
@property(nonatomic, retain)AVPlayer* player;
@property(nonatomic)BOOL force;
-(void)play:(NSURL*)videoURL shouldForce:(BOOL)force;
-(void)stop;
@end

@interface GMediaClass : NSObject <UINavigationControllerDelegate, UIImagePickerControllerDelegate, UIPopoverControllerDelegate>
@property(nonatomic, retain)UIPopoverController *popover;
@property(nonatomic, retain)GMediaView *videoView;
-(id)init;
-(void)deinit;
-(BOOL)isCameraAvailable;
-(void)takePicture;
-(void)takeScreenshot;
-(void)getPicture;
-(void)addToGallery:(NSString*)path;
-(void)playVideo:(NSString*)path shouldForce:(BOOL)force;
@end
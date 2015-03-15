//
//  Copyright (c) 2013 Tap for Tap. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@class TFTAppWall;

@protocol TFTAppWallDelegate <NSObject>
@optional
- (void)tftAppWallDidReceiveAd:(TFTAppWall *)appWall;
- (void)tftAppWall:(TFTAppWall *)appWall didFail:(NSString *)reason;
- (void)tftAppWallDidShow:(TFTAppWall *)appWall;
- (void)tftAppWallWasTapped:(TFTAppWall *)appWall;
- (void)tftAppWallWasDismissed:(TFTAppWall *)appWall;
@end

@interface TFTAppWall : NSObject
- (void)load;
- (void)showWithViewController:(UIViewController *)viewController;
- (void)showAndLoadWithViewController:(UIViewController *)viewController;
- (BOOL)readyToShow;

+ (TFTAppWall *) appWall;
+ (TFTAppWall *) appWallWithDelegate:(id<TFTAppWallDelegate>) delegate;
@end

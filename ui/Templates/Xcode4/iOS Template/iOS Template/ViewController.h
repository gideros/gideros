//
//  ViewController.h
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <UIKit/UIKit.h>

#import <OpenGLES/EAGL.h>

#import <OpenGLES/ES1/gl.h>
#import <OpenGLES/ES1/glext.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>

#import "EAGLView.h"

@interface ViewController : UIViewController<UITableViewDelegate, UITableViewDataSource>
{
    UITableView *tableView;
	    
    BOOL animating;
    BOOL resized;
    NSInteger animationFrameInterval;
    CADisplayLink *displayLink;

	EAGLView* glView;
    
    BOOL statusBarHidden;
    UIStatusBarStyle statusBarStyle;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic, assign) NSInteger animationFrameInterval;
@property (nonatomic) EAGLView* glView;
@property (nonatomic) UITableView *tableView;
@property (nonatomic) CADisplayLink *displayLink;

- (void)startAnimation;
- (void)stopAnimation;
- (void)addProject:(NSString*)project;
- (void)initTable;
- (void)showTable;
- (void)hideTable;

@end

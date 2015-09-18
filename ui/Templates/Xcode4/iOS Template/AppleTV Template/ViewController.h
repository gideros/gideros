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
    EAGLContext *context;
    UITableView *tableView;
    
    BOOL animating;
    NSInteger animationFrameInterval;
    CADisplayLink *displayLink;
    
    EAGLView* glView;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;
@property (nonatomic, assign) EAGLView* glView;
@property (nonatomic, assign) UITableView *tableView;

- (void)startAnimation;
- (void)stopAnimation;
- (void)addProject:(NSString*)project;
- (void)initTable;
- (void)showTable;
- (void)hideTable;

@end

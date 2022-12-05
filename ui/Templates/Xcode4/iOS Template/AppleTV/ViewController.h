//
//  ViewController.h
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "EAGLView.h"

@interface ViewController : UIViewController<UITableViewDelegate, UITableViewDataSource>
{
    UITableView *tableView;
    
    BOOL animating;
    BOOL resized;
    NSInteger animationFrameInterval;
    CADisplayLink *displayLink;
    
    EAGLView* glView;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;
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

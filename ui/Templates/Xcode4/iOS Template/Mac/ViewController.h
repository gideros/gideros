//
//  ViewController.h
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <CoreVideo/CoreVideo.h>
#import "EAGLView.h"

@interface ViewController : NSViewController<NSTableViewDelegate, NSTableViewDataSource>
{
    NSTableView *tableView;
	    
    BOOL animating;
    NSInteger animationFrameInterval;
    CVDisplayLink *displayLink;

	EAGLView* glView;
}

@property (readonly, nonatomic, getter=isAnimating) BOOL animating;
@property (nonatomic) NSInteger animationFrameInterval;
@property (nonatomic, assign) NSTableView *tableView;

- (void)startAnimation;
- (void)stopAnimation;
- (void)addProject:(NSString*)project;
- (void)initTable;
- (void)showTable;
- (void)hideTable;

@end

//
//  ViewController.m
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "ViewController.h"
#import "EAGLView.h"

#include "giderosapi.h"

@interface ViewController ()
@property (nonatomic, assign) CADisplayLink *displayLink;
@end

@implementation ViewController

NSMutableArray *tableData;

@synthesize animating, displayLink, glView, tableView;

- (id)init
{
    tableData = [[NSMutableArray alloc] init];
	if (self = [super init])
	{
		animating = FALSE;
		animationFrameInterval = 1;
		self.displayLink = nil;		
	}
	
	return self;
}

- (void)loadView
{
	self.glView = [[EAGLView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	self.glView.clearsContextBeforeDrawing = NO;
	self.glView.multipleTouchEnabled = YES;
	self.glView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	
	UIView* rootView = [[UIView alloc] initWithFrame:[[UIScreen mainScreen] bounds]];       
	[rootView addSubview:glView];   
	
	self.view = rootView;
}

- (void)viewDidLoad
{
    [self.glView setup];
}

- (void)dealloc
{
    [tableData release];
    
    [super dealloc];
}

- (void)viewWillAppear:(BOOL)animated
{
    [self startAnimation];
    
    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self stopAnimation];
    
    [super viewWillDisappear:animated];
}

- (void)viewDidUnload
{
	[super viewDidUnload];

    [self.glView tearDown];
}

- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    /*
	 Frame interval defines how many display frames must pass between each time the display link fires.
	 The display link will only fire 30 times a second when the frame internal is two on a display that refreshes 60 times a second. The default frame interval setting of one will fire 60 times a second when the display refreshes at 60 times a second. A frame interval setting of less than one results in undefined behavior.
	 */
    if (frameInterval >= 1)
    {
        animationFrameInterval = frameInterval;
        
        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
    if (!animating)
    {
		CADisplayLink *aDisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawFrame)];
        [aDisplayLink setFrameInterval:animationFrameInterval];
        [aDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        self.displayLink = aDisplayLink;
        
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        [self.displayLink invalidate];
        self.displayLink = nil;
        animating = FALSE;
    }
}

- (void)drawFrame
{
    gdr_drawFrame();
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
	gdr_didReceiveMemoryWarning();
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
	return gdr_shouldAutorotateToInterfaceOrientation(interfaceOrientation);
}

-(void)willRotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation duration:(NSTimeInterval)duration
{
	gdr_willRotateToInterfaceOrientation(toInterfaceOrientation);
}

- (void)didRotateFromInterfaceOrientation:(UIInterfaceOrientation)fromInterfaceOrientation
{
	gdr_didRotateFromInterfaceOrientation(fromInterfaceOrientation);
}

- (NSUInteger)supportedInterfaceOrientations
{
    /*NSArray *supportedOrientations = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"UISupportedInterfaceOrientations"];
     NSUInteger result = gdr_supportedInterfaceOrientations();
     BOOL supported = false;
     for (NSNumber *orientation in supportedOrientations) {
     if((result & [orientation integerValue]) != 0){
     supported = true;
     break;
     }
     }
     if(supported)
     return result;
     else
     return UIInterfaceOrientationMaskAll;*/
    return gdr_supportedInterfaceOrientations();

}

- (void)initTable{
    CGRect bounds = [[UIScreen mainScreen] bounds];
    self.tableView=[[UITableView alloc]init];
    self.tableView.frame = CGRectMake(0,0,bounds.size.width,bounds.size.height);
    self.tableView.dataSource=self;
    self.tableView.delegate=self;
    self.tableView.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;
    [self.tableView registerClass:[UITableViewCell class] forCellReuseIdentifier:@"Cell"];
    
    UIView *headerView = [[UIView alloc] initWithFrame:CGRectMake(0, 0, bounds.size.width, 100)];
    UIButton *button = [UIButton buttonWithType:UIButtonTypeRoundedRect];
    [button addTarget:self
               action:@selector(hideTable)
     forControlEvents:UIControlEventTouchUpInside];
    [button setTitle:@"Back" forState:UIControlStateNormal];
    button.frame = CGRectMake(0.0, 30.0, 80.0, 40.0);
    [headerView  addSubview:button];
    UILabel *labelView = [[UILabel alloc] initWithFrame:CGRectMake(120, 0, 200, 100)];
    [labelView setText:@"Gideros Projects"];
    labelView.font=[labelView.font fontWithSize:25];
    [headerView addSubview:labelView];
    self.tableView.tableHeaderView = headerView;
    [labelView release];
    [headerView release];
    [self.tableView reloadData];
    [self.view addSubview:self.tableView];
    [self.tableView setHidden:true];
}

- (void)showTable{
    if(self.tableView){
        [self.tableView setHidden:false];
    }
}

- (void)hideTable{
    if(self.tableView){
        [self.tableView setHidden:true];
    }
}

- (void)addProject:(NSString*)project{
    [tableData addObject:project];
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{

    return [tableData count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *simpleTableIdentifier = @"SimpleTableItem";
    
    UITableViewCell *cell = [self.tableView dequeueReusableCellWithIdentifier:simpleTableIdentifier];
    
    if (cell == nil) {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault reuseIdentifier:simpleTableIdentifier];
    }
    cell.backgroundView = [[UIView alloc] init];
    [cell.backgroundView setBackgroundColor:[UIColor whiteColor]];
    [[[cell contentView] subviews] makeObjectsPerformSelector:@selector(removeFromSuperview)];
    cell.textLabel.text = [tableData objectAtIndex:indexPath.row];
    return cell;
}

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    [self hideTable];
    gdr_openProject([tableData objectAtIndex:indexPath.row]);
}

@end

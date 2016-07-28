
#import <QuartzCore/QuartzCore.h>
#import "CameraCapturePipeline.h"
#include "camerabinder.h"

@interface CameraPluginController () <CameraCapturePipelineDelegate>
{
	BOOL _addedObservers;
	UIBackgroundTaskIdentifier _backgroundRecordingID;
	BOOL _allowedToUseGPU;
}

@property(nonatomic, retain) CameraCapturePipeline *capturePipeline;

@end

@implementation CameraPluginController

- (void)dealloc
{
	if ( _addedObservers ) {
		[[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationDidEnterBackgroundNotification object:[UIApplication sharedApplication]];
		[[NSNotificationCenter defaultCenter] removeObserver:self name:UIApplicationWillEnterForegroundNotification object:[UIApplication sharedApplication]];
		[[NSNotificationCenter defaultCenter] removeObserver:self name:UIDeviceOrientationDidChangeNotification object:[UIDevice currentDevice]];
		[[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
	}

	[_capturePipeline release];
    
    [super dealloc];
}

#pragma mark - View lifecycle

- (void)applicationDidEnterBackground
{
	// Avoid using the GPU in the background
	_allowedToUseGPU = NO;
}

- (void)applicationWillEnterForeground
{
	_allowedToUseGPU = YES;
	self.capturePipeline.renderingEnabled = YES;
}

- (void)start
{
    self.capturePipeline = [[[CameraCapturePipeline alloc] init] autorelease];
    [self.capturePipeline setDelegate:self callbackQueue:dispatch_get_main_queue()];
	
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(applicationDidEnterBackground)
												 name:UIApplicationDidEnterBackgroundNotification
											   object:[UIApplication sharedApplication]];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(applicationWillEnterForeground)
												 name:UIApplicationWillEnterForegroundNotification
											   object:[UIApplication sharedApplication]];
	[[NSNotificationCenter defaultCenter] addObserver:self
											 selector:@selector(deviceOrientationDidChange)
												 name:UIDeviceOrientationDidChangeNotification
											   object:[UIDevice currentDevice]];
	
    // Keep track of changes to the device orientation so we can update the capture pipeline
    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	
	_addedObservers = YES;
	
	// the willEnterForeground and didEnterBackground notifications are subsequently used to update _allowedToUseGPU
	_allowedToUseGPU = ( [UIApplication sharedApplication].applicationState != UIApplicationStateBackground );
	self.capturePipeline.renderingEnabled = _allowedToUseGPU;
	
}

- (void)stop
{
	[self.capturePipeline stopRunning];
}

- (void)deviceOrientationDidChange
{
    UIDeviceOrientation deviceOrientation = [UIDevice currentDevice].orientation;
	
	// Update recording orientation if device changes to portrait or landscape orientation (but not face up/down)
	if ( UIDeviceOrientationIsPortrait( deviceOrientation ) || UIDeviceOrientationIsLandscape( deviceOrientation ) ) {
        [self.capturePipeline setRecordingOrientation:(AVCaptureVideoOrientation)deviceOrientation];
	}
}

- (void)capturePipeline:(CameraCapturePipeline *)capturePipeline didStopRunningWithError:(NSError *)error
{
}

// Preview
- (void)capturePipeline:(CameraCapturePipeline *)capturePipeline previewPixelBufferReadyForDisplay:(CVPixelBufferRef)pixelBuffer
{
	if ( ! _allowedToUseGPU ) {
		return;
	}

	size_t frameWidth = CVPixelBufferGetWidth(pixelBuffer);
    size_t frameHeight = CVPixelBufferGetHeight(pixelBuffer);
    CVOpenGLESTextureRef texture = NULL;
    CVReturn err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                videoTextureCache,
                                                                pixelBuffer,
                                                                NULL,
                                                                GL_TEXTURE_2D,
                                                                GL_RGBA,
                                                                frameWidth,
                                                                frameHeight,
                                                                GL_BGRA,
                                                                GL_UNSIGNED_BYTE,
                                                                0,
                                                                &texture);
	if (texture)
	{
		GLuint glid=CVOpenGLESTextureGetName(texture);
		cameraplugin::cameraTexture->SetNative(&glid); 
	}
}

- (void)capturePipelineDidRunOutOfPreviewBuffers:(CameraCapturePipeline *)capturePipeline
{
}


@end

static CameraController *ctrl=NULL;
void cameraplugin::start()
{
	if (!ctrl)
		ctrl=[[CameraController alloc] init]
	[ctrl start];
}

void cameraplugin::stop()
{
	if (ctrl)
	{
	 [ctrl stop]
	 [ctrl release];
	 ctrl=NULL;
	}
}

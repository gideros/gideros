
#import <QuartzCore/QuartzCore.h>
#import "CameraCapturePipeline.h"
#import <UIKit/UIKit.h>
#include "camerabinder.h"
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include "graphicsbase.h"

@interface CameraPluginController : NSObject <CameraCapturePipelineDelegate>
{
	BOOL _addedObservers;
	UIBackgroundTaskIdentifier _backgroundRecordingID;
	BOOL _allowedToUseGPU;
    CVOpenGLESTextureCacheRef videoTextureCache;
	CVPixelBufferRef videoBuffer;
    ShaderBuffer *rdrTgt;
    TextureData *tex;
    VertexBuffer<unsigned short> indices;
    VertexBuffer<Point2f> vertices;
    VertexBuffer<Point2f> texcoords;
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

- init
{
    self = [super init];
    if ( self )
    {
    videoTextureCache=NULL;
    videoBuffer=NULL;
    EAGLContext *context=[EAGLContext currentContext];
    CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, (__bridge CVEAGLContext) context, NULL,&videoTextureCache);
    if (err)
    {
        NSAssert(NO, @"Error at CVOpenGLESTextureCacheCreate %d",err);
    }
    tex=NULL;
    rdrTgt=NULL;
    self.capturePipeline = [[[CameraCapturePipeline alloc] init] autorelease];
    [self.capturePipeline setDelegate:self callbackQueue:dispatch_get_main_queue()];
        indices.resize(4);
        vertices.resize(4);
        texcoords.resize(4);
        
        indices[0] = 0;
        indices[1] = 1;
        indices[2] = 3;
        indices[3] = 2;
        indices.Update();
        
        texcoords[0] = Point2f(0, 0);
        texcoords[1] = Point2f(1, 0);
        texcoords[2] = Point2f(1, 1);
        texcoords[3] = Point2f(0, 1);
        texcoords.Update();

    }
    return self;
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

- (void)start:(TextureData *)texture o:(int) orientation cw:(int *)camwidth ch:(int *)camheight device:(NSString *) dev
{
    tex = texture;
    rdrTgt = ShaderEngine::Engine->createRenderTarget(tex->id());
    vertices[0] = Point2f(0, 0);
    vertices[1] = Point2f(tex->width, 0);
    vertices[2] = Point2f(tex->width, tex->height);
    vertices[3] = Point2f(0, tex->height);
    vertices.Update();
	
    if (!_addedObservers)
    {
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
    }
    
	// the willEnterForeground and didEnterBackground notifications are subsequently used to update _allowedToUseGPU
	_allowedToUseGPU = ( [UIApplication sharedApplication].applicationState != UIApplicationStateBackground );
    self.capturePipeline.renderingEnabled = _allowedToUseGPU;
    self.capturePipeline.camdev = dev;
    [self.capturePipeline startRunning];
    [self.capturePipeline getVideoWidth:camwidth andHeight:camheight];
    AVCaptureVideoOrientation avo=self.capturePipeline.videoOrientation;
    int ao=0;
    switch (avo) {
        case AVCaptureVideoOrientationPortrait: ao=0; break;
        case AVCaptureVideoOrientationLandscapeLeft: ao=270; break;
        case AVCaptureVideoOrientationPortraitUpsideDown: ao=180; break;
        case AVCaptureVideoOrientationLandscapeRight: ao=90; break;
    }
    ao=(orientation-ao+360)%360;
    int x0=0;
    int x1=1;
    if (self.capturePipeline.frontFacing) { x0=1; x1=0; }
    switch (ao)
    {
        case 0:
            texcoords[0] = Point2f(x0, 0);
            texcoords[1] = Point2f(x1, 0);
            texcoords[2] = Point2f(x1, 1);
            texcoords[3] = Point2f(x0, 1);
            break;
        case 90:
            texcoords[0] = Point2f(x1, 0);
            texcoords[1] = Point2f(x1, 1);
            texcoords[2] = Point2f(x0, 1);
            texcoords[3] = Point2f(x0, 0);
            break;
        case 180:
            texcoords[0] = Point2f(x1, 1);
            texcoords[1] = Point2f(x0, 1);
            texcoords[2] = Point2f(x0, 0);
            texcoords[3] = Point2f(x1, 0);
            break;
        case 270:
            texcoords[0] = Point2f(x0, 1);
            texcoords[1] = Point2f(x0, 0);
            texcoords[2] = Point2f(x1, 0);
            texcoords[3] = Point2f(x1, 1);
            break;
    }
    texcoords.Update();
    switch (ao)
    {
        case 90:
        case 270:
        {
            int tmp=*camwidth;
            *camwidth=*camheight;
            *camheight=tmp;
        }
            break;
        default:
            break;
    }
}

- (void)stop
{
	[self.capturePipeline stopRunning];
    @synchronized(self) {
    if (videoBuffer)
    {
        CFRelease(videoBuffer);
        videoBuffer=NULL;
    }
    if (tex)
    {
    	tex = NULL;
    	delete rdrTgt;
    	rdrTgt=NULL;
    }
    }
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

    @synchronized (self) {
        if (videoBuffer==NULL) {
        	CFRetain(pixelBuffer);
        	videoBuffer=pixelBuffer;
        }
    }
}

- (void) renderCamera
{
	bool hasBuffer=false;
	@synchronized (self) {
    	hasBuffer=(videoBuffer!=NULL);
    }
    if (hasBuffer)
    {
        size_t frameWidth = CVPixelBufferGetWidth(videoBuffer);
        size_t frameHeight = CVPixelBufferGetHeight(videoBuffer);
    	CVOpenGLESTextureRef texture = NULL;
    	CVReturn err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                videoTextureCache,
                                                                videoBuffer,
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
            ShaderEngine::Engine->reset();
            ShaderBuffer *oldfbo = ShaderEngine::Engine->setFramebuffer(rdrTgt);
            ShaderEngine::Engine->setViewport(0, 0, tex->width, tex->height);
            Matrix4 projection = ShaderEngine::Engine->setOrthoFrustum(0,
                                                                       tex->baseWidth, 0, tex->baseHeight, -1, 1);
            ShaderEngine::Engine->setProjection(projection);
            Matrix4 model;
            ShaderEngine::Engine->setModel(model);
            ShaderEngine::Engine->clearColor(0,0,0,0);
                        
            GLuint glid=CVOpenGLESTextureGetName(texture);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, glid);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            ShaderProgram::stdTexture->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 2,
                            &vertices[0], vertices.size(), vertices.modified,
                            &vertices.bufferCache);
            ShaderProgram::stdTexture->setData(ShaderProgram::DataTexture, ShaderProgram::DFLOAT, 2,
                            &texcoords[0], texcoords.size(), texcoords.modified,
                            &texcoords.bufferCache);
            ShaderProgram::stdTexture->drawElements(ShaderProgram::TriangleStrip, indices.size(),
                                 ShaderProgram::DUSHORT, &indices[0], indices.modified,
                                 &indices.bufferCache);
            vertices.modified = false;
            texcoords.modified = false;
            indices.modified = false;
            glBindTexture(GL_TEXTURE_2D, 0);
            
            ShaderEngine::Engine->setFramebuffer(oldfbo);
            CFRelease(texture);
        }
            

		@synchronized (self) {
			CFRelease(videoBuffer);
			videoBuffer=NULL;
    	}
    }
}

- (void)capturePipelineDidRunOutOfPreviewBuffers:(CameraCapturePipeline *)capturePipeline
{
}


@end

static CameraPluginController *ctrl=NULL;
void cameraplugin::init()
{
    if (!ctrl)
        ctrl=[[CameraPluginController alloc] init];
}

std::vector<cameraplugin::CameraDesc> cameraplugin::availableDevices()
{
	std::vector<cameraplugin::CameraDesc> cams;
    NSArray *devices=[AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
    for (int k=0;k<[devices count];k++)
    {
        cameraplugin::CameraDesc cd;
        AVCaptureDevice *ad=[devices objectAtIndex:k];
        cd.name=[ad.uniqueID UTF8String];
        cd.description=[ad.localizedName UTF8String];
        cd.pos=cameraplugin::CameraDesc::POS_UNKNOWN;
        if (ad.position==AVCaptureDevicePositionBack)
            cd.pos=cameraplugin::CameraDesc::POS_BACKFACING;
        if (ad.position==AVCaptureDevicePositionFront)
            cd.pos=cameraplugin::CameraDesc::POS_FRONTFACING;
        cams.push_back(cd);
    }
    
	return cams;
}

void cameraplugin::start(Orientation orientation,int *camwidth,int *camheight,const char *device)
{
    int o=0;
    switch (orientation)
    {
        case ePortrait:
            o=0;
            break;
        case eLandscapeLeft:
            o=90;
            break;
        case ePortraitUpsideDown:
            o=180;
            break;
        case eLandscapeRight:
            o=270;
            break;
    }

    NSString *dev=NULL;
    if (device)
        dev=[NSString stringWithUTF8String:device];
    [ctrl start:cameraplugin::cameraTexture->data o:o cw:camwidth ch:camheight device:dev];
}

void cameraplugin::stop()
{
    [ctrl stop];
}

void cameraplugin::deinit()
{
    if (ctrl)
    {
        [ctrl stop];
        [ctrl release];
        ctrl=NULL;
    }
}

extern "C" void cameraplugin_render()
{
    if (ctrl)
        [ctrl renderCamera];
}

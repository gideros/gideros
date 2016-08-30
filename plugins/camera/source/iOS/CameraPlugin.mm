
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
    CVOpenGLESTextureRef videoTexture,curTexture;
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
    EAGLContext *context=[EAGLContext currentContext];
    CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, (__bridge CVEAGLContext) context, NULL,&videoTextureCache);
    if (err)
    {
        NSAssert(NO, @"Error at CVOpenGLESTextureCacheCreate %d",err);
    }
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

- (void)start:(TextureData *)texture o:(int) orientation cw:(int *)camwidth ch:(int *)camheight
{
    tex = texture;
    rdrTgt = ShaderEngine::Engine->createRenderTarget(tex->id());
    vertices[0] = Point2f(0, 0);
    vertices[1] = Point2f(tex->width, 0);
    vertices[2] = Point2f(tex->width, tex->height);
    vertices[3] = Point2f(0, tex->height);
    vertices.Update();
    switch (orientation)
    {
        case 0: //Portrait
            texcoords[3] = Point2f(1, 1);
            texcoords[0] = Point2f(0, 1);
            texcoords[1] = Point2f(0, 0);
            texcoords[2] = Point2f(1, 0);
            break;
        case 90: //Landscape left
            texcoords[0] = Point2f(1, 1);
            texcoords[1] = Point2f(0, 1);
            texcoords[2] = Point2f(0, 0);
            texcoords[3] = Point2f(1, 0);
            break;
        case 180: //Portrait upside down
            texcoords[1] = Point2f(1, 1);
            texcoords[2] = Point2f(0, 1);
            texcoords[3] = Point2f(0, 0);
            texcoords[0] = Point2f(1, 0);
            break;
        case 270: //Landscape right
            texcoords[2] = Point2f(1, 1);
            texcoords[3] = Point2f(0, 1);
            texcoords[0] = Point2f(0, 0);
            texcoords[1] = Point2f(1, 0);
            break;
    }
    texcoords.Update();

	
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
    [self.capturePipeline startRunning];
    [self.capturePipeline getVideoWidth:camwidth andHeight:camheight];
}

- (void)stop
{
	[self.capturePipeline stopRunning];
    @synchronized(self) {
    if (videoTexture)
    {
        CFRelease(videoTexture);
        videoTexture=NULL;
    }
    if (curTexture)
    {
        CFRelease(curTexture);
        curTexture=NULL;
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
        @synchronized (self) {
        if (videoTexture)
            CFRelease(videoTexture);
        videoTexture=texture;
        }
	}
}

- (void) renderCamera
{
    @synchronized (self) {
        if (videoTexture)
        {
            ShaderEngine::Engine->reset();
            ShaderBuffer *oldfbo = ShaderEngine::Engine->setFramebuffer(rdrTgt);
            ShaderEngine::Engine->setViewport(0, 0, tex->width, tex->height);
            Matrix4 projection = ShaderEngine::Engine->setOrthoFrustum(0,
                                                                       tex->baseWidth, 0, tex->baseHeight, -1, 1);
            ShaderEngine::Engine->setProjection(projection);
            Matrix4 model;
            ShaderEngine::Engine->setModel(model);
            
            GLuint glid=CVOpenGLESTextureGetName(videoTexture);
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
            
            ShaderEngine::Engine->setFramebuffer(oldfbo);

            if (curTexture)
                CFRelease(curTexture);
            curTexture=videoTexture;
            videoTexture=NULL;
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

void cameraplugin::start(Orientation orientation,int *camwidth,int *camheight)
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

    [ctrl start:cameraplugin::cameraTexture->data o:o cw:camwidth ch:camheight];
}

void cameraplugin::stop()
{
    [ctrl stop];
}

void cameraplugin::deinit()
{
    if (ctrl)
    {
        [ctrl release];
        ctrl=NULL;
    }
}

extern "C" void cameraplugin_render()
{
    if (ctrl)
        [ctrl renderCamera];
}


#import <QuartzCore/QuartzCore.h>
#import "CameraCapturePipeline.h"
#import <UIKit/UIKit.h>
#include "camerabinder.h"
//#define GLES
#ifdef GLES
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#else
#import "Metal/Metal.h"
#endif
#include <CoreVideo/CoreVideo.h>
#include "graphicsbase.h"

#ifndef GLES
extern id<MTLDevice> metalDevice;
#endif

static g_id gid=g_NextId();
@interface CameraPluginController : NSObject <CameraCapturePipelineDelegate>
{
	BOOL _addedObservers;
	UIBackgroundTaskIdentifier _backgroundRecordingID;
	BOOL _allowedToUseGPU;
#ifdef GLES
    CVOpenGLESTextureCacheRef videoTextureCache;
#else
    CVMetalTextureCacheRef videoTextureCache;
#endif
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
#ifdef GLES
EAGLContext *context=[EAGLContext currentContext];
CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, (__bridge CVEAGLContext) context, NULL,&videoTextureCache);
if (err)
    NSAssert(NO, @"Error at CVOpenGLESTextureCacheCreate %d",err);
#else
    CVReturn err = CVMetalTextureCacheCreate(kCFAllocatorDefault, NULL, metalDevice, NULL,&videoTextureCache);
    if (err)
        NSAssert(NO, @"Error at CVMetalTextureCacheCreate %d",err);
#endif
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

- (void)setOrientation:(AVCaptureVideoOrientation)o
{
    [self.capturePipeline setRecordingOrientation:o];
    [self.capturePipeline setOrientation:o];
}

- (void)start:(TextureData *)texture o:(AVCaptureVideoOrientation) orientation cw:(int *)camwidth ch:(int *)camheight device:(NSString *) dev pw:(int *)picwidth ph:(int *)picheight
{
    tex = texture;
    rdrTgt = ShaderEngine::Engine->createRenderTarget(tex->id());
    vertices[0] = Point2f(0, 0);
    vertices[1] = Point2f(tex->width, 0);
    vertices[2] = Point2f(tex->width, tex->height);
    vertices[3] = Point2f(0, tex->height);
    vertices.Update();
    texcoords[0] = Point2f(0, 0);
    texcoords[1] = Point2f(1, 0);
    texcoords[2] = Point2f(1, 1);
    texcoords[3] = Point2f(0, 1);
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
	
    // Keep track of changes to the device orientation so we can update the capture pipeline
    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
	
	_addedObservers = YES;
    }
    
	// the willEnterForeground and didEnterBackground notifications are subsequently used to update _allowedToUseGPU
	_allowedToUseGPU = ( [UIApplication sharedApplication].applicationState != UIApplicationStateBackground );
    self.capturePipeline.renderingEnabled = _allowedToUseGPU;
    self.capturePipeline.camdev = dev;
    [self.capturePipeline setRecordingOrientation:orientation];
    [self.capturePipeline setOrientation:orientation];
    [self.capturePipeline startRunning];
    [self.capturePipeline getVideoWidth:camwidth andHeight:camheight];
    [self.capturePipeline getStillWidth:picwidth andHeight:picheight];
    /*
    AVCaptureVideoOrientation avo=self.capturePipeline.videoOrientation;
    int ao=0;
    switch (avo) {
        case AVCaptureVideoOrientationPortrait: ao=0; break;
        case AVCaptureVideoOrientationLandscapeLeft: ao=90; break;
        case AVCaptureVideoOrientationPortraitUpsideDown: ao=180; break;
        case AVCaptureVideoOrientationLandscapeRight: ao=270; break;
    }
    ao=(ao-orientation+360)%360;
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
    }*/
}

- (void)queryCamera:(NSString *) dev o:(int) orientation ret:(cameraplugin::CameraInfo *) ci
{
    AVCaptureDevice *videoDevice=NULL;
    if (dev)
        videoDevice= [AVCaptureDevice deviceWithUniqueID:dev];
    if (!videoDevice)
        videoDevice= [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    
    std::set<uint32_t> rset,fset;
    
    for (AVCaptureDeviceFormat *f in videoDevice.formats) {
        CMVideoDimensions d=CMVideoFormatDescriptionGetDimensions(f.formatDescription);
        uint32_t ds=(d.width<<16)|d.height;
        if (rset.find(ds)==rset.end()) {
            ci->previewSizes.push_back(d.width);
            ci->previewSizes.push_back(d.height);
            rset.insert(ds);
        }
        d=f.highResolutionStillImageDimensions;
        ds=(d.width<<16)|d.height;
        if (fset.find(ds)==fset.end()) {
            ci->pictureSizes.push_back(d.width);
            ci->pictureSizes.push_back(d.height);
            fset.insert(ds);
        }
    }
    if ([videoDevice isFlashModeSupported:AVCaptureFlashModeAuto])
        ci->flashModes.push_back(0);
    if ([videoDevice isFlashModeSupported:AVCaptureFlashModeOff])
        ci->flashModes.push_back(1);
    if ([videoDevice isFlashModeSupported:AVCaptureFlashModeOn]) {
        ci->flashModes.push_back(2);
        if ([videoDevice isTorchModeSupported:AVCaptureTorchModeOn])
            ci->flashModes.push_back(3);
    }
    ci->angle=-1;
}

- (BOOL) setFlash:(int) mode {
    return [self.capturePipeline setFlash:mode];
}

- (BOOL) takePicture {
    return [self.capturePipeline takePicture:^(NSData *image) {
        char *event=(char *)malloc(sizeof(int)+[image length]);
        *((int *)event)=(int)[image length];
        [image getBytes:event+sizeof(int) length:[image length]];
        gevent_EnqueueEvent(gid, cameraplugin::callback_s, 2, event, 1, NULL);
    }];
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
    	hasBuffer=(videoBuffer!=nullptr);
    }
    if (hasBuffer)
    {
        size_t frameWidth = CVPixelBufferGetWidth(videoBuffer);
        size_t frameHeight = CVPixelBufferGetHeight(videoBuffer);
#ifdef GLES
    	CVOpenGLESTextureRef texture = NULL;
    	CVReturn err = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                                videoTextureCache,
                                                                videoBuffer,
                                                                nullptr,
                                                                GL_TEXTURE_2D,
                                                                GL_RGBA,
                                                                frameWidth,
                                                                frameHeight,
                                                                GL_BGRA,
                                                                GL_UNSIGNED_BYTE,
                                                                0,
                                                                &texture);
#else
    CVMetalTextureRef texture = nullptr;
    /*CVReturn err = */CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
                                                            videoTextureCache,
                                                            videoBuffer,
                                                            nullptr,
                                                            MTLPixelFormatBGRA8Unorm,
                                                            frameWidth,
                                                            frameHeight,
                                                            0,
                                                            &texture);
#endif
		if (texture)
		{
            ShaderEngine::Engine->reset();
            ShaderBuffer *oldfbo = ShaderEngine::Engine->setFramebuffer(rdrTgt);
            ShaderEngine::Engine->clearColor(0,0,0,0);
            ShaderEngine::Engine->setViewport(0, 0, tex->width, tex->height);
            Matrix4 projection = ShaderEngine::Engine->setOrthoFrustum(0,
                                                                       tex->baseWidth, tex->baseHeight,0, -1, 1,true);
            ShaderEngine::Engine->setProjection(projection);
            Matrix4 model;
            ShaderEngine::Engine->setModel(model);
                        
#ifdef GLES
            GLuint glid=CVOpenGLESTextureGetName(texture);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, glid);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
            ShaderTexture *ctex=ShaderEngine::Engine->createTexture(ShaderTexture::FMT_NATIVE, ShaderTexture::PK_UBYTE, (int)frameWidth, (int)frameHeight, NULL, ShaderTexture::WRAP_CLAMP, ShaderTexture::FILT_LINEAR);
            ctex->setNative(CVMetalTextureGetTexture(texture));
            ShaderEngine::Engine->bindTexture(0, ctex);
#endif
            ShaderProgram::stdTexture->setData(ShaderProgram::DataVertex, ShaderProgram::DFLOAT, 2,
                            &vertices[0], (unsigned int) vertices.size(), vertices.modified,
                            &vertices.bufferCache);
            ShaderProgram::stdTexture->setData(ShaderProgram::DataTexture, ShaderProgram::DFLOAT, 2,
                            &texcoords[0], (unsigned int)texcoords.size(), texcoords.modified,
                            &texcoords.bufferCache);
            ShaderProgram::stdTexture->drawElements(ShaderProgram::TriangleStrip, (unsigned int)indices.size(),
                                 ShaderProgram::DUSHORT, &indices[0], indices.modified,
                                 &indices.bufferCache);
            vertices.modified = false;
            texcoords.modified = false;
            indices.modified = false;
#ifdef GLES
            glBindTexture(GL_TEXTURE_2D, 0);
#else
            delete ctex;
#endif
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

void cameraplugin::start(Orientation orientation,int *camwidth,int *camheight,const char *device, int *picwidth, int *picheight)
{
    AVCaptureVideoOrientation o=AVCaptureVideoOrientationPortrait;
    switch (orientation)
    {
        default:
        case ePortrait:
            o=AVCaptureVideoOrientationPortrait;
            break;
        case eLandscapeLeft:
            o=AVCaptureVideoOrientationLandscapeLeft;
            break;
        case ePortraitUpsideDown:
            o=AVCaptureVideoOrientationPortraitUpsideDown;
            break;
        case eLandscapeRight:
            o=AVCaptureVideoOrientationLandscapeRight;
            break;
    }

    NSString *dev=NULL;
    if (device)
        dev=[NSString stringWithUTF8String:device];
    [ctrl start:cameraplugin::cameraTexture->data o:o cw:camwidth ch:camheight device:dev pw:picwidth ph:picheight];
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

bool cameraplugin::isAvailable()
{
    return true;
}

bool cameraplugin::setFlash(int mode) {
    return [ctrl setFlash:mode];
}

bool cameraplugin::takePicture() {
    return [ctrl takePicture];
}

cameraplugin::CameraInfo cameraplugin::queyCamera(const char *device, Orientation orientation)
{
    int o=0;
    switch (orientation)
    {
        default:
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
    
    cameraplugin::CameraInfo ci;
    [ctrl queryCamera:dev o:o ret:&ci];
    return ci;
}

void cameraplugin::setOrientation(Orientation orientation) {
    AVCaptureVideoOrientation o=AVCaptureVideoOrientationPortrait;
    switch (orientation)
    {
        default:
        case ePortrait:
            o=AVCaptureVideoOrientationPortrait;
            break;
        case eLandscapeLeft:
            o=AVCaptureVideoOrientationLandscapeLeft;
            break;
        case ePortraitUpsideDown:
            o=AVCaptureVideoOrientationPortraitUpsideDown;
            break;
        case eLandscapeRight:
            o=AVCaptureVideoOrientationLandscapeRight;
            break;
    }
    [ctrl setOrientation:o];
}


extern "C" void cameraplugin_render()
{
    if (ctrl)
        [ctrl renderCamera];
}

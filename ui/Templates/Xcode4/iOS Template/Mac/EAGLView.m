//
//  EAGLView.m
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "EAGLView.h"
#import "ViewController.h"

#include "giderosapi.h"
//GIDEROS-TAG-MAC:DRAWDEFS//

id<MTLDevice> metalDevice=nil;
MTLRenderPassDescriptor *metalFramebuffer;
extern void metalShaderEnginePresent(id<MTLDrawable>);
extern void metalShaderNewFrame();

@interface EAGLView (PrivateMethods)
- (void)createFramebuffer;
- (void)deleteFramebuffer;
@end

@interface LuaException : NSException
@end
@implementation LuaException
@end


@implementation EAGLView

// You must implement this method
+ (Class)layerClass
{
    metalDevice= MTLCreateSystemDefaultDevice();
    if (metalDevice)
    return [CAMetalLayer class];
}

- (id)initWithFrame:(CGRect)rect
{
    self = [super initWithFrame:rect];
	if (self)
    {
        if (metalDevice) {
            metalLayer= (CAMetalLayer *)self.layer;
            metalLayer.opaque = TRUE;
            //metalLayer.presentsWithTransaction=YES;
        }
		retinaDisplay = NO;
        _autocorrectionType = UITextAutocorrectionTypeNo;
    }
    
    return self;
}

- (void)dealloc
{
    [self deleteFramebuffer];    
    [context release];
    
    [super dealloc];
}

- (void) setup
{
    if (metalDevice) {
        if (@available (iOS 11, tvOS 11, macOS 10.13, *)) {
            MTLCaptureManager *sharedCaptureManager = [MTLCaptureManager sharedCaptureManager];
            id<MTLCaptureScope> myCaptureScope = [sharedCaptureManager newCaptureScopeWithDevice:metalDevice];
            myCaptureScope.label = @"Gideros Frame";
            sharedCaptureManager.defaultCaptureScope = myCaptureScope;
        }
        metalFramebuffer=[MTLRenderPassDescriptor renderPassDescriptor];
        [metalFramebuffer retain];
    }
    [self setFramebuffer];
}

- (void) tearDown
{
    // Tear down context.
    if (metalDevice) {
    }
}

- (BOOL)canBecomeFirstResponder
{
    return gdr_keyboardVisible();
}

static int lfbw=-1,lfbh=-1;
- (void)createFramebuffer
{
    if (metalDevice)
    {
        if (!metalDrawable) {
            if (@available (iOS 11, tvOS 11, macOS 10.13, *))
                [[MTLCaptureManager sharedCaptureManager].defaultCaptureScope beginScope];

            metalDrawable=[metalLayer nextDrawable];
            [metalDrawable retain];
            if (metalDepth==nil) {
                MTLTextureDescriptor *td=[MTLTextureDescriptor new];
                td.pixelFormat=MTLPixelFormatDepth32Float;
                td.width=[metalDrawable.texture width];
                td.height=[metalDrawable.texture height];
                td.usage=MTLTextureUsageRenderTarget;
                metalDepth=[metalDevice newTextureWithDescriptor:td];
                [metalDepth retain];
                td.pixelFormat=MTLPixelFormatStencil8;
                metalStencil=[metalDevice newTextureWithDescriptor:td];
                [metalStencil retain];
            }
            metalFramebuffer.colorAttachments[0].texture=metalDrawable.texture;
            metalFramebuffer.depthAttachment.texture=metalDepth;
            metalFramebuffer.depthAttachment.loadAction=MTLLoadActionClear;
            metalFramebuffer.stencilAttachment.texture=metalStencil;
            metalFramebuffer.stencilAttachment.loadAction=MTLLoadActionClear;
            framebufferWidth=[metalDrawable.texture width];
            framebufferHeight=[metalDrawable.texture height];
            metalFramebuffer.colorAttachments[0].loadAction=MTLLoadActionClear;
            metalFramebuffer.colorAttachments[0].clearColor=MTLClearColorMake(1,1,1,1);
            if ((lfbw!=framebufferWidth)||(lfbh!=framebufferHeight))
                gdr_surfaceChanged(framebufferWidth,framebufferHeight);
            lfbw=framebufferWidth;
            lfbh=framebufferHeight;
            metalShaderNewFrame();
        }
    }
}

- (void)deleteFramebuffer
{
    if (metalDevice)
    {
        [metalDrawable release];
        metalDrawable=nil;
        [metalDepth release];
        metalDepth=nil;
        [metalStencil release];
        metalStencil=nil;
        lfbw=-1;
        lfbh=-1;
    }
    framebufferDirty=FALSE;
}

- (void)setFramebuffer
{
   if (framebufferDirty)
            [self deleteFramebuffer];
   if (metalDevice)
    {
        if (metalDevice) {
        }
        if (metalDevice&&(!metalDrawable))
            [self createFramebuffer];
//GIDEROS-TAG-MAC:PREDRAW//
    }
}

- (BOOL)presentFramebuffer
{
    BOOL success = FALSE;
    
    if (metalDevice)
    {
        if (metalDevice) {
            metalShaderEnginePresent(metalDrawable);
            [metalDrawable release];
            metalDrawable=nil;
            if (@available (iOS 11, tvOS 11, macOS 10.13, *))
                [[MTLCaptureManager sharedCaptureManager].defaultCaptureScope endScope];
            
            [self createFramebuffer];
            success= TRUE;
        }
        //GIDEROS-TAG-MAC:POSTDRAW//
    }
    
    return success;
}

- (void)layoutSubviews
{
    if (@available (iOS 11,*)) {
        UIEdgeInsets sa=[[self superview] safeAreaInsets];
        safeArea.origin.x=sa.left;
        safeArea.origin.y=sa.top;
        safeArea.size.height=sa.bottom;
        safeArea.size.width=sa.right;
    }
    else
        safeArea=CGRectMake(0,0,0,0);
    
    // The framebuffer will be re-created at the beginning of the next setFramebuffer method call.
    CGSize drawableSize = self.bounds.size;
    drawableSize.width *= self.contentScaleFactor;
    drawableSize.height *= self.contentScaleFactor;
    metalLayer.drawableSize = drawableSize;
    framebufferDirty=TRUE;
}

- (void) getSafeArea:(CGRect *) sa
{
    *sa=safeArea;
}

- (void)enableRetinaDisplay:(BOOL)enable
{
	if (retinaDisplay == enable)
		return;
	
	if ([[UIScreen mainScreen] respondsToSelector:@selector(scale)] == NO)
		return;
	
	if ([self respondsToSelector:@selector(contentScaleFactor)] == NO)
		return;
	
	retinaDisplay = enable;
	
	if (retinaDisplay)
		self.contentScaleFactor = [UIScreen mainScreen].scale;
	else 
		self.contentScaleFactor = 1;
	
    // The framebuffer will be re-created (with the new resolution) at the beginning of the next setFramebuffer method call.
    CGSize drawableSize = self.bounds.size;
    drawableSize.width *= self.contentScaleFactor;
    drawableSize.height *= self.contentScaleFactor;
    metalLayer.drawableSize = drawableSize;
    framebufferDirty=TRUE;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    if(!gdr_isRunning()){
        ViewController* view = (ViewController*)[self.superview nextResponder];
        [view showTable];
    }
    gdr_touchesBegan(touches, [event allTouches]);
}
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    gdr_touchesMoved(touches, [event allTouches]);
}
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    gdr_touchesEnded(touches, [event allTouches]);
}
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    gdr_touchesCancelled(touches, [event allTouches]);
}

- (void)insertText:(NSString *)text;
{
    gdr_keyChar(text);
}

- (void)deleteBackward;
{
    gdr_keyDown(8,0); //Simulate a backspace key press and release
    gdr_keyUp(8,0);
}

- (BOOL) hasText
{
    return TRUE;
}

- (void) reportLuaError:(NSString *)error
{
    //GIDEROS-TAG-IOS:LUAERROR//
    @throw [[LuaException alloc] initWithName:@"Lua" reason:error userInfo:nil];
}

@end

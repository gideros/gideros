//
//  EAGLView.m
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "EAGLView.h"
#import "ViewController.h"

#define UIView NSView
#include "giderosapi.h"
//GIDEROS-TAG-MAC:DRAWDEFS//

id<MTLDevice> metalDevice=nil;
MTLRenderPassDescriptor *metalFramebuffer;
extern void metalShaderEnginePresent(id<MTLDrawable>);
extern void metalShaderNewFrame(void);

@interface EAGLView (PrivateMethods)
- (void)createFramebuffer;
- (void)deleteFramebuffer;
@end

@interface LuaException : NSException
@end
@implementation LuaException
@end


@implementation EAGLView

- (id)initWithFrame:(CGRect)rect
{
    self = [super initWithFrame:rect];
    self.wantsLayer = YES;
    self.layer = [CAMetalLayer layer];
    metalDevice= MTLCreateSystemDefaultDevice();
    metalLayer= (CAMetalLayer *)self.layer;
    metalLayer.opaque = TRUE;
    metalLayer.device=metalDevice;
        //metalLayer.presentsWithTransaction=YES;
    retinaDisplay = NO;
    modifiers=0;
    
    [self setAcceptsTouchEvents:TRUE];
    //by using [self bounds] we get our internal origin (0, 0)
    NSTrackingArea* trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:(NSTrackingMouseEnteredAndExited | NSTrackingActiveAlways) owner:self userInfo:nil];
    [self addTrackingArea:trackingArea];
    
    return self;
}

- (BOOL) isFlipped {
    return TRUE;
}

- (void)dealloc
{
    [self deleteFramebuffer];    
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
    }
    [self setFramebuffer];
}

- (void) tearDown
{
    // Tear down context.
}

- (BOOL)acceptsFirstResponder
{
    return TRUE;
}

static NSUInteger lfbw=0,lfbh=0;
- (void)createFramebuffer
{
    if (metalDevice)
    {
        if (!metalDrawable) {
            if (@available (iOS 11, tvOS 11, macOS 10.13, *))
                [[MTLCaptureManager sharedCaptureManager].defaultCaptureScope beginScope];

            metalDrawable=[metalLayer nextDrawable];
            if (metalDepth==nil) {
                MTLTextureDescriptor *td=[MTLTextureDescriptor new];
                td.pixelFormat=MTLPixelFormatDepth32Float_Stencil8;
                td.width=[metalDrawable.texture width];
                td.height=[metalDrawable.texture height];
                td.storageMode=MTLStorageModePrivate;
                td.usage=MTLTextureUsageRenderTarget;
                metalDepth=[metalDevice newTextureWithDescriptor:td];
            }
            metalFramebuffer.colorAttachments[0].texture=metalDrawable.texture;
            metalFramebuffer.depthAttachment.texture=metalDepth;
            metalFramebuffer.depthAttachment.loadAction=MTLLoadActionClear;
            metalFramebuffer.stencilAttachment.texture=metalDepth;
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
        if (metalDrawable) {
            metalShaderEnginePresent(metalDrawable);
            metalDrawable=nil;
            if (@available (iOS 11, tvOS 11, macOS 10.13, *))
                [[MTLCaptureManager sharedCaptureManager].defaultCaptureScope endScope];
        }
        metalDrawable=nil;
        metalDepth=nil;
        lfbw=0;
        lfbh=0;
    }
    framebufferDirty=FALSE;
}

- (void)setFramebuffer
{
   if (framebufferDirty)
        [self deleteFramebuffer];
   if (metalDevice&&(!metalDrawable))
        [self createFramebuffer];
//GIDEROS-TAG-MAC:PREDRAW//
}

- (BOOL)presentFramebuffer
{
    BOOL success = FALSE;
    
    if (metalDevice)
    {
        metalShaderEnginePresent(metalDrawable);
        metalDrawable=nil;
        if (@available (iOS 11, tvOS 11, macOS 10.13, *))
            [[MTLCaptureManager sharedCaptureManager].defaultCaptureScope endScope];
        
        [self createFramebuffer];
        success= TRUE;
        //GIDEROS-TAG-MAC:POSTDRAW//
    }
    
    return success;
}

- (void)resized
{
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

- (void)enableRetinaDisplay:(BOOL)enable scalePtr:(float *)scale
{
    enable=TRUE;
    if (retinaDisplay == enable)
        return;

    retinaDisplay = enable;
    
    if (retinaDisplay)
        self.contentScaleFactor = self.window.backingScaleFactor;
    else
        self.contentScaleFactor = 1;
    
    // The framebuffer will be re-created (with the new resolution) at the beginning of the next setFramebuffer method call.
    CGSize drawableSize = self.bounds.size;
    drawableSize.width *= self.contentScaleFactor;
    drawableSize.height *= self.contentScaleFactor;
    metalLayer.drawableSize = drawableSize;
    framebufferDirty=TRUE;
    *scale=self.contentScaleFactor;
}

int mouseButton(NSInteger bn) {
    if (bn) return 1<<bn;
    return 0;
}

int keyMods(NSEventModifierFlags mod) {
    int rmod=0;
    if (mod&NSEventModifierFlagShift) rmod|=1;
    if (mod&NSEventModifierFlagOption) rmod|=2;
    if (mod&NSEventModifierFlagControl) rmod|=4;
    if (mod&NSEventModifierFlagCommand) rmod|=8;
    return rmod;
}

- (void)mouseDown:(NSEvent *)event
{
    if (event.window==nil) return;
    NSPoint event_location = event.locationInWindow;
    NSPoint local_point = [self convertPoint:event_location fromView:nil];
    gdr_mouseDown(local_point.x, local_point.y, mouseButton(event.buttonNumber), keyMods(event.modifierFlags));
}

- (void)mouseUp:(NSEvent *)event
{
    if (event.window==nil) return;
    NSPoint event_location = event.locationInWindow;
    NSPoint local_point = [self convertPoint:event_location fromView:nil];
    gdr_mouseUp(local_point.x, local_point.y, mouseButton(event.buttonNumber), keyMods(event.modifierFlags));
}

- (void)mouseMoved:(NSEvent *)event
{
    if (event.window==nil) return;
    NSPoint event_location = event.locationInWindow;
    NSPoint local_point = [self convertPoint:event_location fromView:nil];
    gdr_mouseHover(local_point.x, local_point.y, mouseButton(event.buttonNumber), keyMods(event.modifierFlags));
}

- (void)mouseDragged:(NSEvent *)event
{
    if (event.window==nil) return;
    NSPoint event_location = event.locationInWindow;
    NSPoint local_point = [self convertPoint:event_location fromView:nil];
    gdr_mouseMove(local_point.x, local_point.y, mouseButton(event.buttonNumber), keyMods(event.modifierFlags));
}

- (void)mouseEntered:(NSEvent *)event
{
    if (event.window==nil) return;
    NSPoint event_location = event.locationInWindow;
    NSPoint local_point = [self convertPoint:event_location fromView:nil];
    gdr_mouseEnter(local_point.x, local_point.y, mouseButton(event.buttonNumber), keyMods(event.modifierFlags));
}

- (void)mouseExited:(NSEvent *)event
{
    if (event.window==nil) return;
    NSPoint event_location = event.locationInWindow;
    NSPoint local_point = [self convertPoint:event_location fromView:nil];
    gdr_mouseLeave(local_point.x, local_point.y, keyMods(event.modifierFlags));
}

- (void)scrollWheel:(NSEvent *)event
{
    if (event.window==nil) return;
    NSPoint event_location = event.locationInWindow;
    NSPoint local_point = [self convertPoint:event_location fromView:nil];
    gdr_mouseWheel(local_point.x, local_point.y, mouseButton(event.buttonNumber),event.scrollingDeltaY*120, keyMods(event.modifierFlags));
}

- (void)touchesBegan:(NSSet *)touches withEvent:(NSEvent *)event
{
    if (@available(macOS 10.12, *))
        gdr_touchesBegan(touches, [event allTouches]);
}
- (void)touchesMoved:(NSSet *)touches withEvent:(NSEvent *)event
{
    if (@available(macOS 10.12, *))
        gdr_touchesMoved(touches, [event allTouches]);
}
- (void)touchesEnded:(NSSet *)touches withEvent:(NSEvent *)event
{
    if (@available(macOS 10.12, *))
        gdr_touchesEnded(touches, [event allTouches]);
}
- (void)touchesCancelled:(NSSet *)touches withEvent:(NSEvent *)event
{
    if (@available(macOS 10.12, *))
        gdr_touchesCancelled(touches, [event allTouches]);
}

- (void)keyDown:(NSEvent *)event
{
	int mods=keyMods(event.modifierFlags);
    gdr_keyDown(event.keyCode, mods, [event isARepeat]?1:0);
    NSString *c=event.characters;
    if ((c!=NULL)&&((mods&~1)==0)) {
        unichar uni=[c characterAtIndex:0];
        if ((uni<0xE000)||(uni>=0xF800)) {
            gdr_keyChar(c);
        }
    }
}

- (void)keyUp:(NSEvent *)event
{
    gdr_keyUp(event.keyCode, keyMods(event.modifierFlags), [event isARepeat]?1:0);
}

- (void)flagsChanged:(NSEvent *)event
{
    NSEventModifierFlags set=event.modifierFlags&(~modifiers);
    NSEventModifierFlags clr=modifiers&(~event.modifierFlags);
    if (set&NSEventModifierFlagShift) gdr_keyDown(16, 0, 0);
    if (clr&NSEventModifierFlagShift) gdr_keyUp(16, 0, 0);
    if (set&NSEventModifierFlagControl) gdr_keyDown(17, 0, 0);
    if (clr&NSEventModifierFlagControl) gdr_keyUp(17, 0, 0);
    if (set&NSEventModifierFlagOption) gdr_keyDown(18, 0, 0);
    if (clr&NSEventModifierFlagOption) gdr_keyUp(18, 0, 0);
    modifiers=event.modifierFlags;
}

- (void)insertText:(NSString *)text;
{
    gdr_keyChar(text);
}

- (void)deleteBackward;
{
    gdr_keyDown(0x33,0, 0); //Simulate a backspace key press and release
    gdr_keyUp(0x33,0, 0);
}

- (BOOL) hasText
{
    return TRUE;
}

- (void) reportLuaError:(NSString *)error
{
    //GIDEROS-TAG-MAC:LUAERROR//
    @throw [[LuaException alloc] initWithName:@"Lua" reason:error userInfo:nil];
}

@end

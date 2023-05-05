//
//  EAGLView.m
//
//  Copyright 2012 Gideros Mobile. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "EAGLView.h"
#import "ViewController.h"

#include "giderosapi.h"
//GIDEROS-TAG-IOS:DRAWDEFS//

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
    //[self setFramebuffer];
}

- (void) tearDown
{
    // Tear down context.
    }

- (BOOL)canBecomeFirstResponder
{
    return gdr_keyboardVisible();
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
   if (metalDevice)
    {
        if (!metalDrawable)
            [self createFramebuffer];
        
        //GIDEROS-TAG-IOS:PREDRAW//
        //glViewport(0, 0, framebufferWidth, framebufferHeight);
    }
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
        //GIDEROS-TAG-IOS:POSTDRAW//
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
    gdr_keyDown(0x33,0,0); //Simulate a backspace key press and release
    gdr_keyUp(0x33,0,0);
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

/* Methods for manipulating text. */
- (nullable NSString *)textInRange:(UITextRange *)range
{
    return NULL;
}

- (void)replaceRange:(UITextRange *)range withText:(NSString *)text
{
}

- (void)setMarkedText:(nullable NSString *)markedText selectedRange:(NSRange)selectedRange // selectedRange is a range within the markedText
{
    
}
- (void)unmarkText
{
    
}

/* Methods for creating ranges and positions. */
- (nullable UITextRange *)textRangeFromPosition:(UITextPosition *)fromPosition toPosition:(UITextPosition *)toPosition
{
    return NULL;
}

- (nullable UITextPosition *)positionFromPosition:(UITextPosition *)position offset:(NSInteger)offset
{
    return NULL;
}

- (nullable UITextPosition *)positionFromPosition:(UITextPosition *)position inDirection:(UITextLayoutDirection)direction offset:(NSInteger)offset
{
    return NULL;
}

/* Simple evaluation of positions */
- (NSComparisonResult)comparePosition:(UITextPosition *)position toPosition:(UITextPosition *)other {
    return 0;
}
- (NSInteger)offsetFromPosition:(UITextPosition *)from toPosition:(UITextPosition *)toPosition {
    return 0;
}

/* Layout questions. */
- (nullable UITextPosition *)positionWithinRange:(UITextRange *)range farthestInDirection:(UITextLayoutDirection)direction
{
    return NULL;
}

- (nullable UITextRange *)characterRangeByExtendingPosition:(UITextPosition *)position inDirection:(UITextLayoutDirection)direction
{
    return NULL;
}

/* Writing direction */
- (UITextWritingDirection)baseWritingDirectionForPosition:(UITextPosition *)position inDirection:(UITextStorageDirection)direction {
    return UITextWritingDirectionLeftToRight;
}
- (void)setBaseWritingDirection:(UITextWritingDirection)writingDirection forRange:(UITextRange *)range
{
    
}

/* Geometry used to provide, for example, a correction rect. */
- (CGRect)firstRectForRange:(UITextRange *)range
{
    return CGRectMake(0,0,0,0);
}
- (CGRect)caretRectForPosition:(UITextPosition *)position
{
    return CGRectMake(0,0,0,0);
}
- (NSArray *)selectionRectsForRange:(UITextRange *)range NS_AVAILABLE_IOS(6_0)       // Returns an array of UITextSelectionRects
{
    return NULL;
}

/* Hit testing. */
- (nullable UITextPosition *)closestPositionToPoint:(CGPoint)point
{
    return NULL;
}
- (nullable UITextPosition *)closestPositionToPoint:(CGPoint)point withinRange:(UITextRange *)range
{
    return NULL;
}
- (nullable UITextRange *)characterRangeAtPoint:(CGPoint)point
{
    return NULL;
}


@end

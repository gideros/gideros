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

@dynamic context;

// You must implement this method
+ (Class)layerClass
{
#ifdef GIDEROS_METAL
    metalDevice= MTLCreateSystemDefaultDevice();
    if (metalDevice)
    return [CAMetalLayer class];
#else
    metalDevice=false;
#endif
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)rect
{
    self = [super initWithFrame:rect];
	if (self)
    {
        if (metalDevice) {
#ifdef GIDEROS_METAL
            metalLayer= (CAMetalLayer *)self.layer;
            metalLayer.opaque= TRUE;
            //metalLayer.presentsWithTransaction=YES;
#endif
        }
        else {
        eaglLayer = (CAEAGLLayer *)self.layer;
        
        eaglLayer.opaque = TRUE;
        eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                        [NSNumber numberWithBool:FALSE], kEAGLDrawablePropertyRetainedBacking,
                                        kEAGLColorFormatRGBA8, kEAGLDrawablePropertyColorFormat,
                                        nil];
            }
		retinaDisplay = NO;
        _autocorrectionType = UITextAutocorrectionTypeNo;
    }
    
    return self;
}

- (void)dealloc
{
    [self deleteFramebuffer];    
    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
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
    else
    {
    EAGLContext *aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    
    if (!aContext)
        NSLog(@"Failed to create ES context");
    else if (![EAGLContext setCurrentContext:aContext])
        NSLog(@"Failed to set ES context current");
    
    self.context = aContext;
    [aContext release];
    
    [self setContext:context];
    }
    [self setFramebuffer];
}

- (void) tearDown
{
    // Tear down context.
    if (metalDevice) {
    }
    else {
        if ([EAGLContext currentContext] == context)
            [EAGLContext setCurrentContext:nil];
    }
}

- (BOOL)canBecomeFirstResponder
{
    return gdr_keyboardVisible();
}

- (EAGLContext *)context
{
    return context;
}

- (void)setContext:(EAGLContext *)newContext
{
    if (context != newContext)
    {
        [self deleteFramebuffer];
        
        [context release];
        context = [newContext retain];
        
        [EAGLContext setCurrentContext:nil];
    }
}

static int lfbw=-1,lfbh=-1;
- (void)createFramebuffer
{
    if (metalDevice)
    {
#ifdef GIDEROS_METAL
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
#endif
    }
    else {
    if (context && !defaultFramebuffer)
    {
        [EAGLContext setCurrentContext:context];
        
        // Create default framebuffer object.
        glGenFramebuffers(1, &defaultFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        
        // Create color render buffer and allocate backing store.
        glGenRenderbuffers(1, &colorRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
        [context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &framebufferWidth);
        glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &framebufferHeight);
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorRenderbuffer);
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        gdr_surfaceChanged(framebufferWidth,framebufferHeight);
    }
}
}

- (void)deleteFramebuffer
{
    if (metalDevice)
    {
#ifdef GIDEROS_METAL
        [metalDrawable release];
        metalDrawable=nil;
#endif
        [metalDepth release];
        metalDepth=nil;
        [metalStencil release];
        metalStencil=nil;
        lfbw=-1;
        lfbh=-1;
    }
    else {
    if (context)
    {
        [EAGLContext setCurrentContext:context];
        
        if (defaultFramebuffer)
        {
            glDeleteFramebuffers(1, &defaultFramebuffer);
            defaultFramebuffer = 0;
        }
        
        if (colorRenderbuffer)
        {
            glDeleteRenderbuffers(1, &colorRenderbuffer);
            colorRenderbuffer = 0;
        }
    }
    }
    framebufferDirty=FALSE;
}

- (void)setFramebuffer
{
   if (framebufferDirty)
            [self deleteFramebuffer];
   if (metalDevice||context)
    {
        if (metalDevice) {
        }
        if (context)
        [EAGLContext setCurrentContext:context];
        
        if (context&&(!defaultFramebuffer))
            [self createFramebuffer];
#ifdef GIDEROS_METAL
        if (metalDevice&&(!metalDrawable))
            [self createFramebuffer];
#endif
        
        if (context)
        glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);
        
        //GIDEROS-TAG-IOS:PREDRAW//
        //glViewport(0, 0, framebufferWidth, framebufferHeight);
    }
}

- (BOOL)presentFramebuffer
{
    BOOL success = FALSE;
    
    if (metalDevice||context)
    {
        if (metalDevice) {
#ifdef GIDEROS_METAL
            metalShaderEnginePresent(metalDrawable);
            [metalDrawable release];
            metalDrawable=nil;
            if (@available (iOS 11, tvOS 11, macOS 10.13, *))
                [[MTLCaptureManager sharedCaptureManager].defaultCaptureScope endScope];
            
            [self createFramebuffer];
            success= TRUE;
#endif
        }
        else {
        [EAGLContext setCurrentContext:context];
        
        glBindRenderbuffer(GL_RENDERBUFFER, colorRenderbuffer);
        
        success = [context presentRenderbuffer:GL_RENDERBUFFER];
        }
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
#ifdef GIDEROS_METAL
    metalLayer.drawableSize = drawableSize;
#endif
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
#ifdef GIDEROS_METAL
    metalLayer.drawableSize = drawableSize;
#endif
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

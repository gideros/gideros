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

- (id)initWithFrame:(CGRect)rect
{
    NSOpenGLPixelFormatAttribute attrs[]={
        NSOpenGLPFAStencilSize,8,
        NSOpenGLPFADepthSize,16,
        NSOpenGLPFADoubleBuffer,
        0
    };
    self = [super initWithFrame:rect pixelFormat:[[NSOpenGLPixelFormat alloc] initWithAttributes:attrs]];
	if (self)
    {
		retinaDisplay = NO;
        context=[self openGLContext];
        markedText = [[NSMutableAttributedString alloc] init];
        [context makeCurrentContext];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(_surfaceNeedsUpdate:)
                                                     name:NSViewBoundsDidChangeNotification
                                                   object:self];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(_surfaceNeedsUpdate:)
                                                     name:NSViewGlobalFrameDidChangeNotification
                                                   object:self];
    }
    return self;
}


- (BOOL)acceptsFirstResponder {
    return TRUE;
}

- (NSOpenGLContext *)context
{
    return context;
}

- (void)setFramebuffer
{
   if (framebufferDirty)
   {
       gdr_surfaceChanged(framebufferWidth,framebufferHeight);
       framebufferDirty=FALSE;
   }

   if (context)
    {
        [context makeCurrentContext];
        
        //GIDEROS-TAG-IOS:PREDRAW//
        glViewport(0, 0, framebufferWidth, framebufferHeight);
    }
}

- (BOOL)presentFramebuffer
{
    BOOL success = FALSE;
    
    if (context)
    {
 
        [context flushBuffer];
        //GIDEROS-TAG-IOS:POSTDRAW//
    }
    
    return success;
}


- (void) _surfaceNeedsUpdate:(NSNotification*)notification
{
    // The framebuffer will be re-created at the beginning of the next setFramebuffer method call.
    NSRect backingBounds = [self convertRectToBacking:[self bounds]];
    framebufferWidth=backingBounds.size.width;
    framebufferHeight=backingBounds.size.height;
    framebufferDirty=TRUE;
    [context makeCurrentContext];
    [context update];
}

- (void)enableRetinaDisplay:(BOOL)enable
{
}

- (void)deleteBackward;
{
    gdr_keyDown(8,0); //Simulate a backspace key press and release
    gdr_keyUp(8,0);
}

- (void) reportLuaError:(NSString *)error
{
    //GIDEROS-TAG-IOS:LUAERROR//
    @throw [[LuaException alloc] initWithName:@"Lua" reason:error userInfo:nil];
}

int convertButton(NSInteger b)
{
    if (b==0)
        return 1; //GINPUT_LEFT
    if (b==0)
        return 2; //GINPUT_RIGHT
    if (b==0)
        return 3; //GINPUT_MIDDLE
    return 0;
}

int convertPButton(NSUInteger b)
{
    return b;
}

int convertModifier(NSEventModifierFlags b)
{
    return 0;
}

-(NSPoint) convertCoords:(NSPoint) w {
    NSPoint c;
    c.x=w.x;
    c.y=self.bounds.size.height-w.y;
    return c;
}

- (void)mouseDown:(NSEvent *)event;
{
    NSPoint ll=[self convertCoords:event.locationInWindow];
    gdr_mouseDown(ll.x, ll.y, convertButton(event.buttonNumber), convertModifier(event.modifierFlags));
}

- (void)mouseUp:(NSEvent *)event
{
    NSPoint ll=[self convertCoords:event.locationInWindow];
    gdr_mouseUp(ll.x, ll.y, convertButton(event.buttonNumber), convertModifier(event.modifierFlags));
}

- (void)mouseMoved:(NSEvent *)event
{
    NSPoint ll=[self convertCoords:event.locationInWindow];
    gdr_mouseHover(ll.x, ll.y, convertPButton([NSEvent pressedMouseButtons]), convertModifier(event.modifierFlags));
}

- (void)mouseDragged:(NSEvent *)event
{
    NSPoint ll=[self convertCoords:event.locationInWindow];
    gdr_mouseMove(ll.x, ll.y, convertPButton([NSEvent pressedMouseButtons]), convertModifier(event.modifierFlags));
}

- (void)scrollWheel:(NSEvent *)event
{
    NSPoint ll=[self convertCoords:event.locationInWindow];
    gdr_mouseWheel(ll.x, ll.y, convertPButton([NSEvent pressedMouseButtons]), event.scrollingDeltaY, convertModifier(event.modifierFlags));
}

static int convertKey(NSEvent *e) {
    switch (e.keyCode) {
        case 123: return 37;
        case 124: return 39;
        case 125: return 40;
        case 126: return 38;
        case 51: return 8;
        default:
            //NSLog(@"Code: %d %@ %d",e.keyCode,e.characters,[[e.characters uppercaseString] characterAtIndex:0]);
            if ([e.characters length]>0)
                return [[e.characters uppercaseString] characterAtIndex:0];
    }
    return 0;
}

- (void)keyDown:(NSEvent *)event;
{
    int kcode=convertKey(event);
    gdr_keyDown(kcode, event.isARepeat?1:0);
    if (kcode>32)
        [self interpretKeyEvents:[NSArray arrayWithObject:event]];
    else
    {
        char code[2]={kcode,0};
        gdr_keyChar([NSString stringWithUTF8String:code]);
    }
}

- (void)keyUp:(NSEvent *)event;
{
    gdr_keyUp(convertKey(event),0);
}

- (BOOL)hasMarkedText
{
    return (markedText.length > 0);
}

- (NSRange)markedRange
{
    return (markedText.length > 0) ?
    NSMakeRange(0, markedText.length-1) : NSMakeRange(0,0);
}

- (NSRange)selectedRange
{
    return NSMakeRange(0,0);
}

- (void)setMarkedText:(id)aString
        selectedRange:(NSRange)selectedRange
     replacementRange:(NSRange)replacementRange
{
    if( [aString isKindOfClass: [NSAttributedString class]] )
    {
        [markedText initWithAttributedString: aString];
    }
    else
    {
        [markedText initWithString: aString];
    }
}

- (void)unmarkText
{
    [[markedText mutableString] setString:@""];
}

- (NSArray*)validAttributesForMarkedText
{
    return [NSArray array];
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)aRange
                                               actualRange:(NSRangePointer)actualRange
{
    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)aPoint
{
    return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)aRange
                         actualRange:(NSRangePointer)actualRange
{
    return NSMakeRect(0, 0, 0, 0);
}

- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange
{
    NSString* characters;
    if ([aString isKindOfClass: [NSAttributedString class]]) {
        characters = [aString string];
    } else {
        characters = (NSString*)aString;
    }
    if ([characters length]>0)
        gdr_keyChar(characters);

}

@end

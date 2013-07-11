//
//  ESRenderer.h
//  iphoneplayer
//
//  Created by Atilim Cetin on 2/28/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>

class LuaApplication;

@protocol ESRenderer <NSObject>

- (void) render;
- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer;
- (LuaApplication*) getLuaApplication;

@end

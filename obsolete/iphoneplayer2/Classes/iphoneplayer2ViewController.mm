//
//  iphoneplayer2ViewController.m
//  iphoneplayer2
//
//  Created by Atilim Cetin on 12/7/10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <QuartzCore/QuartzCore.h>

#import "iphoneplayer2ViewController.h"
#import "EAGLView.h"

#include <pthread.h>
#include <semaphore.h>

#include <vector>
#include <set>

#include "uitouchmanager.h"

#include <platform.h>
#include <luaapplication.h>
#include <libnetwork.h>
#include <soundimplementation.h>
#include <soundsystemopenal.h>
#include <sounddecoderwav.h>
#include <sounddecoderavaudioplayer.h>

#include <sys/stat.h>	// header for mkdir function


static pthread_mutex_t eventMutex;
static pthread_t gameThreadHandle;
static sem_t* sem;
static bool gameLoopActive = true;
static iphoneplayer2ViewController* that = NULL;

static Server* g_server = NULL;
static void printToServer(const char* str, void* data)
{
	unsigned int size = 1 + strlen(str) + 1;
	char* buffer = (char*)malloc(size);
	
	int pos = 0;
	buffer[pos] = 4;
	pos += 1;
	strcpy(buffer + pos, str);
	pos += strlen(str) + 1;
	
	g_server->sendData(buffer, size);
	
	free(buffer);
}


void* gameThread(void* args)
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	
	printf("starting game thread\n");
	
//	[NSThread setThreadPriority:0.5];
	
	sem_wait(sem);
	
	printf("starting game loop\n");
	
	while (gameLoopActive)
	{
		//printf(".");
		int deltaFrame = 1;
		
		sem_wait(sem);
		
		// drain any extra values in the semaphore
		while (sem_trywait(sem) != -1)
		{
			deltaFrame += 1;
		}
		
//		if (deltaFrame > 1)
//			printf("deltaFrame: %d\n", deltaFrame);
		
		pthread_mutex_lock(&eventMutex);
		[that drawFrame];
		pthread_mutex_unlock(&eventMutex);
	}
	
	printf("ending game loop\n");
	
	[pool release];
	
	return NULL;
}


// Uniform index.
enum {
    UNIFORM_TRANSLATE,
    NUM_UNIFORMS
};
GLint uniforms[NUM_UNIFORMS];

// Attribute index.
enum {
    ATTRIB_VERTEX,
    ATTRIB_COLOR,
    NUM_ATTRIBUTES
};

@interface iphoneplayer2ViewController ()
@property (nonatomic, retain) EAGLContext *context;
@property (nonatomic, assign) CADisplayLink *displayLink;
- (BOOL)loadShaders;
- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file;
- (BOOL)linkProgram:(GLuint)prog;
- (BOOL)validateProgram:(GLuint)prog;
@end

@implementation iphoneplayer2ViewController

@synthesize animating, context, displayLink;

- (void)awakeFromNib
{
    //EAGLContext *aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
	EAGLContext *aContext = NULL;
    
    if (!aContext)
    {
        aContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
    }
    
    if (!aContext)
        NSLog(@"Failed to create ES context");
    else if (![EAGLContext setCurrentContext:aContext])
        NSLog(@"Failed to set ES context current");
    
	self.context = aContext;
	[aContext release];
	
    [(EAGLView *)self.view setContext:context];
    [(EAGLView *)self.view setFramebuffer];
    
    if ([context API] == kEAGLRenderingAPIOpenGLES2)
        [self loadShaders];
    
    animating = FALSE;
    animationFrameInterval = 1;
    self.displayLink = nil;
	

	
	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
	NSString* documentsDirectory = [paths objectAtIndex:0];
	printf("%s\n", [documentsDirectory UTF8String]);
	
	NSString* temporaryDirectory = NSTemporaryDirectory();
	printf("%s\n", [temporaryDirectory UTF8String]);
	
	setDocumentsDirectory([documentsDirectory UTF8String]);
	setTemporaryDirectory([temporaryDirectory UTF8String]);
	setResourceDirectory([documentsDirectory UTF8String]);
	
	soundImplementation_ = new SoundImplementation;
	soundImplementation_->addSoundSystem<SoundSystemOpenAL>();
	
	setSoundInterface(soundImplementation_);
	
	initializeSound();
	
	SoundDecoder* wav = soundImplementation_->addSoundDecoder<SoundDecoderWav>(1);
	soundImplementation_->addExtension("wav", wav);
	
	SoundDecoder* mp3 = soundImplementation_->addSoundDecoder<SoundDecoderAVAudioPlayer>(0x80000000);
	soundImplementation_->addExtension("mp3", mp3);
	
	
	server_ = new Server(15000);
	g_server = server_;
	
	application_ = new LuaApplication;
	application_->setPrintFunc(printToServer);
	application_->enableExceptions();
	application_->initialize();		
	
	
	
	uiTouchManager_ = new UITouchManager;	

	UIAccelerometer* accelerometer = [UIAccelerometer sharedAccelerometer];
	accelerometer.delegate = self;
	accelerometer.updateInterval = 1.0 / 60.0;
	
	
	that = self;
	if (pthread_mutex_init(&eventMutex, NULL) == -1)
	{
		perror("pthread_mutex_init");
	}
	[NSThread setThreadPriority:1.0];
	sem_unlink("gameLoopSemaphore");
	sem = sem_open("gameLoopSemaphore", O_CREAT, S_IRWXU, 0);
	pthread_create(&gameThreadHandle, NULL, gameThread, NULL);	
}

-(void)exitGameLoop
{
	gameLoopActive = false;
	sem_post(sem);
	void* thread_return = 0;
	pthread_join(gameThreadHandle, &thread_return);
	
	sem_close(sem);
	sem_unlink("gameLoopSemaphore");
}

- (void)dealloc
{
    if (program)
    {
        glDeleteProgram(program);
        program = 0;
    }
	
	application_->deinitialize();
	delete application_;
	application_ = 0;    
		
    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
    
    [context release];

	fileList_.clear();

	deinitializeSound();
	setSoundInterface(NULL);
	delete soundImplementation_;
	
	delete server_;
	server_ = 0;
	g_server = 0;
	
	delete uiTouchManager_;  
    
	[super dealloc];
}

- (void)viewWillAppear:(BOOL)animated
{
    [self startAnimation];
    
    [super viewWillAppear:animated];
}

- (void)viewWillDisappear:(BOOL)animated
{
    [self stopAnimation];
    
    [super viewWillDisappear:animated];
}

- (void)viewDidUnload
{
	[super viewDidUnload];
	
    if (program)
    {
        glDeleteProgram(program);
        program = 0;
    }

    // Tear down context.
    if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
	self.context = nil;	
}

- (NSInteger)animationFrameInterval
{
    return animationFrameInterval;
}

- (void)setAnimationFrameInterval:(NSInteger)frameInterval
{
    /*
	 Frame interval defines how many display frames must pass between each time the display link fires.
	 The display link will only fire 30 times a second when the frame internal is two on a display that refreshes 60 times a second. The default frame interval setting of one will fire 60 times a second when the display refreshes at 60 times a second. A frame interval setting of less than one results in undefined behavior.
	 */
    if (frameInterval >= 1)
    {
        animationFrameInterval = frameInterval;
        
        if (animating)
        {
            [self stopAnimation];
            [self startAnimation];
        }
    }
}

- (void)startAnimation
{
    if (!animating)
    {
        CADisplayLink *aDisplayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(postFrame:)];
        [aDisplayLink setFrameInterval:animationFrameInterval];
        [aDisplayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
        self.displayLink = aDisplayLink;
        
        animating = TRUE;
    }
}

- (void)stopAnimation
{
    if (animating)
    {
        [self.displayLink invalidate];
        self.displayLink = nil;
        animating = FALSE;
    }
}


- (void) postFrame:(id)sender
{
	sem_post(sem);
}


- (void)drawFrame
{
	int dataTotal = 0;
	
	while (true)
	{
		int dataSent0 = server_->dataSent();
		int dataReceived0 = server_->dataReceived();
		
		NetworkEvent event;
		server_->tick(&event);
		
		//		if (event.eventCode != eNone)
		//			printf("%s\n", eventCodeString(event.eventCode));
		
		int dataSent1 = server_->dataSent();
		int dataReceived1 = server_->dataReceived();
		
		if (event.eventCode == eDataReceived)
		{
			const std::vector<char>& data = event.data;
			
			switch (data[0])
			{
				case 0:			// create folder
				{
					std::string folderName = &data[1];
					mkdir(pathForFile(folderName.c_str()), 0755);
					break;
				}
					
				case 1:			// create file
				{
					std::string fileName = &data[1];
					FILE* fos = fopen(pathForFile(fileName.c_str()), "wb");
					int pos = 1 + fileName.size() + 1;
					if (data.size() > pos)
						fwrite(&data[pos], data.size() - pos, 1, fos);
					fclose(fos);
					fileList_.push_back(fileName);
					break;
				}
				case 2:
				{
					printf("play message is received\n");
					
					try
					{
						application_->deinitialize();
						application_->initialize();
						for (std::size_t i = 0; i < fileList_.size(); ++i)
						{
							if (fileList_[i].size() >= 5)
							{
								std::string ext = fileList_[i].substr(fileList_[i].size() - 4);
								
								for (std::size_t j = 0; j < ext.size(); ++j)
									ext[j] = tolower(ext[j]);
								
								if (ext == ".lua")
									application_->loadFile(fileList_[i].c_str());
							}
						}
					}
					catch (LuaException e)
					{
						printf("%s\n", e.what());
					}
					
					break;
				}
				case 3:
				{
					printf("stop message is received\n");
					application_->deinitialize();
					application_->initialize();
					break;
				}
				case 5:
				{
					//					deleteFiles();
					fileList_.clear();
					break;
				}				
			}
		}
		
		int dataDelta = (dataSent1 - dataSent0) + (dataReceived1 - dataReceived0);
		dataTotal += dataDelta;
		
		if (dataDelta == 0 || dataTotal > 1024)
			break;
	}
	
    
	
	
	[(EAGLView *)self.view setFramebuffer];
    

	try
	{
		application_->renderScene();
	}
	catch (LuaException e)
	{
		printf("%s\n", e.what());
	}	
	
/*
	// Replace the implementation of this method to do your own custom drawing.
    static const GLfloat squareVertices[] = {
        -0.5f, -0.33f,
        0.5f, -0.33f,
        -0.5f,  0.33f,
        0.5f,  0.33f,
    };
    
    static const GLubyte squareColors[] = {
        255, 255,   0, 255,
        0,   255, 255, 255,
        0,     0,   0,   0,
        255,   0, 255, 255,
    };
    
    static float transY = 0.0f;
    
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if ([context API] == kEAGLRenderingAPIOpenGLES2)
    {
        // Use shader program.
        glUseProgram(program);
        
        // Update uniform value.
        glUniform1f(uniforms[UNIFORM_TRANSLATE], (GLfloat)transY);
        transY += 0.075f;	
        
        // Update attribute values.
        glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices);
        glEnableVertexAttribArray(ATTRIB_VERTEX);
        glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, 1, 0, squareColors);
        glEnableVertexAttribArray(ATTRIB_COLOR);
        
        // Validate program before drawing. This is a good check, but only really necessary in a debug build.
        // DEBUG macro must be defined in your debug configurations if that's not already the case.
#if defined(DEBUG)
        if (![self validateProgram:program])
        {
            NSLog(@"Failed to validate program: %d", program);
            return;
        }
#endif
    }
    else
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslatef(0.0f, (GLfloat)(sinf(transY)/2.0f), 0.0f);
        transY += 0.075f;
        
        glVertexPointer(2, GL_FLOAT, 0, squareVertices);
        glEnableClientState(GL_VERTEX_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, 0, squareColors);
        glEnableClientState(GL_COLOR_ARRAY);
    }
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
*/
    
    [(EAGLView *)self.view presentFramebuffer];
}

- (void)didReceiveMemoryWarning
{
    // Releases the view if it doesn't have a superview.
    [super didReceiveMemoryWarning];
    
    // Release any cached data, images, etc. that aren't in use.
}

- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file
{
    GLint status;
    const GLchar *source;
    
    source = (GLchar *)[[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] UTF8String];
    if (!source)
    {
        NSLog(@"Failed to load vertex shader");
        return FALSE;
    }
    
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);
    
#if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        NSLog(@"Shader compile log:\n%s", log);
        free(log);
    }
#endif
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0)
    {
        glDeleteShader(*shader);
        return FALSE;
    }
    
    return TRUE;
}

- (BOOL)linkProgram:(GLuint)prog
{
    GLint status;
    
    glLinkProgram(prog);
    
#if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program link log:\n%s", log);
        free(log);
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0)
        return FALSE;
    
    return TRUE;
}

- (BOOL)validateProgram:(GLuint)prog
{
    GLint logLength, status;
    
    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0)
    {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0)
        return FALSE;
    
    return TRUE;
}

- (BOOL)loadShaders
{
    GLuint vertShader, fragShader;
    NSString *vertShaderPathname, *fragShaderPathname;
    
    // Create shader program.
    program = glCreateProgram();
    
    // Create and compile vertex shader.
    vertShaderPathname = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"vsh"];
    if (![self compileShader:&vertShader type:GL_VERTEX_SHADER file:vertShaderPathname])
    {
        NSLog(@"Failed to compile vertex shader");
        return FALSE;
    }
    
    // Create and compile fragment shader.
    fragShaderPathname = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"fsh"];
    if (![self compileShader:&fragShader type:GL_FRAGMENT_SHADER file:fragShaderPathname])
    {
        NSLog(@"Failed to compile fragment shader");
        return FALSE;
    }
    
    // Attach vertex shader to program.
    glAttachShader(program, vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(program, fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(program, ATTRIB_VERTEX, "position");
    glBindAttribLocation(program, ATTRIB_COLOR, "color");
    
    // Link program.
    if (![self linkProgram:program])
    {
        NSLog(@"Failed to link program: %d", program);
        
        if (vertShader)
        {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader)
        {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (program)
        {
            glDeleteProgram(program);
            program = 0;
        }
        
        return FALSE;
    }
    
    // Get uniform locations.
    uniforms[UNIFORM_TRANSLATE] = glGetUniformLocation(program, "translate");
    
    // Release vertex and fragment shaders.
    if (vertShader)
        glDeleteShader(vertShader);
    if (fragShader)
        glDeleteShader(fragShader);
    
    return TRUE;
}


- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	NSArray* touchesArray = [touches allObjects];
	static std::vector<UITouch*> uiTouches;
	uiTouches.clear();
	for (int i = 0; i < [touchesArray count]; ++i)
	{
		UITouch* touch = [touchesArray objectAtIndex:i];
		uiTouches.push_back(touch);
	}
	
	NSArray* allTouchesArray = [[event allTouches] allObjects];
	static std::vector<UITouch*> uiAllTouches;
	uiAllTouches.clear();
	for (int i = 0; i < [allTouchesArray count]; ++i)
	{
		UITouch* touch = [allTouchesArray objectAtIndex:i];
		uiAllTouches.push_back(touch);
	}
	
	
	static std::vector<Touch*> touchesSet;
	static std::vector<Touch*> allTouchesSet;
	touchesSet.clear();
	allTouchesSet.clear();
	
	uiTouchManager_->update(uiTouches, uiAllTouches, &touchesSet, &allTouchesSet);
	
	
//	printf("touch pool size: %d\n", uiTouchManager_->poolSize());

	pthread_mutex_lock(&eventMutex);
	try
	{
		application_->touchesBegan(touchesSet, allTouchesSet);
	}
	catch (LuaException e)
	{
		printf("%s\n", e.what());
	}
	
	pthread_mutex_unlock(&eventMutex);
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	NSArray* touchesArray = [touches allObjects];
	static std::vector<UITouch*> uiTouches;
	uiTouches.clear();
	for (int i = 0; i < [touchesArray count]; ++i)
	{
		UITouch* touch = [touchesArray objectAtIndex:i];
		uiTouches.push_back(touch);
	}
	
	NSArray* allTouchesArray = [[event allTouches] allObjects];
	static std::vector<UITouch*> uiAllTouches;
	uiAllTouches.clear();
	for (int i = 0; i < [allTouchesArray count]; ++i)
	{
		UITouch* touch = [allTouchesArray objectAtIndex:i];
		uiAllTouches.push_back(touch);
	}
	
	
	static std::vector<Touch*> touchesSet;
	static std::vector<Touch*> allTouchesSet;
	touchesSet.clear();
	allTouchesSet.clear();
	
	uiTouchManager_->update(uiTouches, uiAllTouches, &touchesSet, &allTouchesSet);
	
//	printf("touch pool size: %d\n", uiTouchManager_->poolSize());

	pthread_mutex_lock(&eventMutex);

	try	
	{
		application_->touchesMoved(touchesSet, allTouchesSet);
	}
	catch (LuaException e)
	{
		printf("%s\n", e.what());
	}
 
	pthread_mutex_unlock(&eventMutex);
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	NSArray* touchesArray = [touches allObjects];
	static std::vector<UITouch*> uiTouches;
	uiTouches.clear();
	for (int i = 0; i < [touchesArray count]; ++i)
	{
		UITouch* touch = [touchesArray objectAtIndex:i];
		uiTouches.push_back(touch);
	}
	
	NSArray* allTouchesArray = [[event allTouches] allObjects];
	static std::vector<UITouch*> uiAllTouches;
	uiAllTouches.clear();
	for (int i = 0; i < [allTouchesArray count]; ++i)
	{
		UITouch* touch = [allTouchesArray objectAtIndex:i];
		uiAllTouches.push_back(touch);
	}
	
	
	static std::vector<Touch*> touchesSet;
	static std::vector<Touch*> allTouchesSet;
	touchesSet.clear();
	allTouchesSet.clear();
	
	uiTouchManager_->update(uiTouches, uiAllTouches, &touchesSet, &allTouchesSet);
	
//	printf("touch pool size: %d\n", uiTouchManager_->poolSize());

	pthread_mutex_lock(&eventMutex);
	try
	{
		application_->touchesEnded(touchesSet, allTouchesSet);
	}
	catch (LuaException e)
	{
		printf("%s\n", e.what());
	} 
	pthread_mutex_unlock(&eventMutex);
}


- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	NSArray* touchesArray = [touches allObjects];
	static std::vector<UITouch*> uiTouches;
	uiTouches.clear();
	for (int i = 0; i < [touchesArray count]; ++i)
	{
		UITouch* touch = [touchesArray objectAtIndex:i];
		uiTouches.push_back(touch);
	}
	
	NSArray* allTouchesArray = [[event allTouches] allObjects];
	static std::vector<UITouch*> uiAllTouches;
	uiAllTouches.clear();
	for (int i = 0; i < [allTouchesArray count]; ++i)
	{
		UITouch* touch = [allTouchesArray objectAtIndex:i];
		uiAllTouches.push_back(touch);
	}
	
	
	static std::vector<Touch*> touchesSet;
	static std::vector<Touch*> allTouchesSet;
	touchesSet.clear();
	allTouchesSet.clear();
	
	uiTouchManager_->update(uiTouches, uiAllTouches, &touchesSet, &allTouchesSet);
	
//	printf("touch pool size: %d\n", uiTouchManager_->poolSize());

	pthread_mutex_lock(&eventMutex);
	try
	{
		application_->touchesCancelled(touchesSet, allTouchesSet);
	}
	catch (LuaException e)
	{
		printf("%s\n", e.what());
	} 
	pthread_mutex_unlock(&eventMutex);
}

- (void)accelerometer:(UIAccelerometer *)accelerometer didAccelerate:(UIAcceleration *)acceleration
{
	::setAccelerometer(acceleration.x, acceleration.y, acceleration.z);
}


@end

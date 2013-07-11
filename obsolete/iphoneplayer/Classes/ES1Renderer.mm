//
//  ES1Renderer.m
//  iphoneplayer
//
//  Created by Atilim Cetin on 2/28/10.
//  Copyright __MyCompanyName__ 2010. All rights reserved.
//

#import "ES1Renderer.h"

#include "platform.h"
#include "libnetwork.h"
#include "luaapplication.h"
#include <soundimplementation.h>
#include <soundsystemopenal.h>
#include <sounddecoderwav.h>
#include <sounddecoderavaudioplayer.h>

#include <sys/stat.h>	// header for mkdir function

@implementation ES1Renderer


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

// Create an ES 1.1 context
- (id) init
{
	if (self = [super init])
	{
		context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        if (!context || ![EAGLContext setCurrentContext:context])
		{
            [self release];
            return nil;
        }
		
		// Create default framebuffer object. The backing will be allocated for the current layer in -resizeFromLayer
		glGenFramebuffersOES(1, &defaultFramebuffer);
		glGenRenderbuffersOES(1, &colorRenderbuffer);
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, colorRenderbuffer);
		
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
	}
	
	return self;
}

- (void) render
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
	
	
    // Replace the implementation of this method to do your own custom drawing
    
	
	// This application only creates a single context which is already set current at this point.
	// This call is redundant, but needed if dealing with multiple contexts.
    [EAGLContext setCurrentContext:context];
    
	// This application only creates a single default framebuffer which is already bound at this point.
	// This call is redundant, but needed if dealing with multiple framebuffers.
    glBindFramebufferOES(GL_FRAMEBUFFER_OES, defaultFramebuffer);
    //glViewport(0, 0, backingWidth, backingHeight);
    
	try
	{
		application_->renderScene();
	}
	catch (LuaException e)
	{
		printf("%s\n", e.what());
	}
    
	// This application only creates a single color renderbuffer which is already bound at this point.
	// This call is redundant, but needed if dealing with multiple renderbuffers.
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    [context presentRenderbuffer:GL_RENDERBUFFER_OES];
}

- (BOOL) resizeFromLayer:(CAEAGLLayer *)layer
{	
	// Allocate color buffer backing based on the current layer size
    glBindRenderbufferOES(GL_RENDERBUFFER_OES, colorRenderbuffer);
    [context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:layer];
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &backingWidth);
    glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &backingHeight);
	
    if (glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES) != GL_FRAMEBUFFER_COMPLETE_OES)
	{
		NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES));
        return NO;
    }
    
    return YES;
}


- (LuaApplication*) getLuaApplication
{
	return application_;
}


- (void) dealloc
{
	// Tear down GL
	if (defaultFramebuffer)
	{
		glDeleteFramebuffersOES(1, &defaultFramebuffer);
		defaultFramebuffer = 0;
	}

	if (colorRenderbuffer)
	{
		glDeleteRenderbuffersOES(1, &colorRenderbuffer);
		colorRenderbuffer = 0;
	}
	
	// Tear down context
	if ([EAGLContext currentContext] == context)
        [EAGLContext setCurrentContext:nil];
	
	[context release];
	context = nil;
	
	fileList_.clear();
	
	application_->deinitialize();
	delete application_;
	application_ = 0;
	
	deinitializeSound();
	setSoundInterface(NULL);
	delete soundImplementation_;
	
	delete server_;
	server_ = 0;
	g_server = 0;
	
	[super dealloc];
}

@end

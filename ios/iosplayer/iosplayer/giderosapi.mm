#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#else
#import <UIKit/UIKit.h>
#endif

#include <sys/stat.h>

#include <libnetwork.h>
#include <application.h>
#include <luaapplication.h>
#include <gfile.h>
#include <gfile_p.h>
#include <platformutil.h>
#include <vector>
#include <bytebuffer.h>
#include <ghttp.h>

#include <platform.h>

#include <gtexture.h>

#include <gstdio.h>

#include <stage.h>
#include <splashscreen.h>

#include <gpath.h>

#include <pystring.h>

#include <glog.h>

#include <binder.h>

#include <gvfs-native.h>

#include <gui.h>
#include <ginput.h>
#include <ginput-ios.h>
#include <gevent.h>
#if TARGET_OS_TV == 0
#include <ggeolocation.h>
#endif

#include "giderosapi.h"

#include <pluginmanager.h>

#include <glog.h>

#include <gtexture.h>

#include <gapplication.h>
#include <gapplication-ios.h>

#include <gaudio.h>
#include <debugging.h>

#define THREADED_RENDER_LOOP 1

@interface CustomThread : NSThread

- (id)init;
- (void)main;

@property (nonatomic, assign) void *(*startRoutine)(void *);
@property (nonatomic, assign) void *arg;

@end


@implementation CustomThread

@synthesize startRoutine = startRoutine_;
@synthesize arg = arg_;

- (id)init
{
    if (self = [super init])
    {
        startRoutine_ = NULL;
        arg_ = NULL;
    }
    
    return self;    
}

- (id)initWithStartRoutine:(void *(*)(void *))startRoutine withArg:(void*)arg
{
    if (self = [super init])
    {
        startRoutine_ = startRoutine;
        arg_ = arg;
    }
    
    return self;
}

- (void)main
{
    startRoutine_(arg_);
}

@end


extern "C" {
void g_setFps(int);
int g_getFps();
}
void drawInfo();
void drawInfoMargins(int xm,int ym);
void refreshLocalIPs();
void g_exit();

static volatile const char* licenseKey_ = "9852564f4728e0c11e34ca3eb5fe20b2";
//-----------------------------------------01234567890123456------------------

class ApplicationManager;

class NetworkManager
{
public:
	NetworkManager(ApplicationManager *application);
	~NetworkManager();
	void tick();
    std::string openProject_;
	
	void setResourceDirectory(const char* resourceDirectory)
	{
		resourceDirectory_ = resourceDirectory;
	}
	
	void setMd5FileName(const char *md5FileName)
	{
		md5filename_ = md5FileName;
		loadMD5();
	}
	
	static void printToServer_s(const char *str, int len, void *data)
	{
        NSLog(@"%s", str);
        if (data)
            static_cast<NetworkManager*>(data)->printToServer(str, len);
	}
	
	void printToServer(const char *str, int len)
	{
		unsigned int size = 1 + ((len < 0) ? strlen(str) : len) + 1;
		char* buffer = (char*)malloc(size);
		
    	buffer[0] = 4;
    	memcpy(buffer + 1, str,size-2);
    	buffer[size-1]=0;
        
		server_->sendData(buffer, size, true);
		
		free(buffer);
	}	
	
private:
	void createFolder(const std::vector<char> &data);
	void createFile(const std::vector<char> &data);
	void play(const std::vector<char> &data);
	void stop();
	void sendFileList();
	void setProjectName(const std::vector<char> &data);
	void deleteFile(const std::vector<char> &data);
	void setProperties(const std::vector<char> &data);
	
private:
	std::string md5filename_;
	std::map<std::string, std::vector<unsigned char> > md5_;
	void loadMD5();
	void saveMD5();
	void calculateMD5(const char *file);	
	
private:
	ApplicationManager *application_;
	Server *server_;
	std::string resourceDirectory_;
};


class ApplicationManager
{
public:
	ApplicationManager(UIView *view, int width, int height, bool player);
	~ApplicationManager();
	
    void drawFirstFrame();

	void luaError(const char *msg);
	void forceTick();
	
//	void surfaceCreated();
//	void surfaceChanged(int width, int height);
	void drawFrame(bool force);
	
//	void setDirectories(const char *externalDir, const char *internalDir, const char *cacheDir);
//	void setFileSystem(const char *files);
    
    void openProject(const char* project);
    void setOpenProject(const char* project);
	
	void play(const std::vector<std::string>& luafiles);
	void stop();
	void setProjectName(const char *projectName);
	void setProjectProperties(const ProjectProperties &properties);
    bool isRunning();
	
	void touchesBegan(NSSet *touches, NSSet *allTouches);
	void touchesMoved(NSSet *touches, NSSet *allTouches);
	void touchesEnded(NSSet *touches, NSSet *allTouches);
	void touchesCancelled(NSSet *touches, NSSet *allTouches);
	void touchesReset();

    void keyDown(int keyCode, int mods, int repeat);
    void keyUp(int keyCode, int mods, int repeat);
    void keyChar(NSString *text);

	void suspend();
	void resume();
	
#if THREADED_RENDER_LOOP
	static void *renderLoop_s(void *args);
	void *renderLoop();
#endif
	
	void exitRenderLoop();
	void exitRenderLoopHelper();
	
	void didReceiveMemoryWarning();
#if !TARGET_OS_TV && !TARGET_OS_OSX
	BOOL shouldAutorotateToInterfaceOrientation(UIInterfaceOrientation interfaceOrientation);	
	void willRotateToInterfaceOrientationHelper(UIInterfaceOrientation toInterfaceOrientation);
	void willRotateToInterfaceOrientation(UIInterfaceOrientation toInterfaceOrientation);
	void didRotateFromInterfaceOrientation(UIInterfaceOrientation fromInterfaceOrientation);
    NSUInteger supportedInterfaceOrientations();
#else
        void willRotateToInterfaceOrientationHelperTV(Orientation toInterfaceOrientation);
#endif
    void requestDeviceOrientation(gapplication_Orientation iO,gapplication_AutoOrientation iAutoRot);
    
    void handleOpenUrl(NSURL *url);
    
    void foreground();
    void background();
    void surfaceChanged(int width,int height);
    
    bool isKeyboardVisible();
    bool setKeyboardVisibility(bool visible);


private:
	void loadProperties();
	void loadLuaFiles();
	void drawIPs();
	int convertKeyCode(int keyCode);
	
private:
	bool player_;
    bool keyboardVisible_;
	LuaApplication *application_;
	NetworkManager *networkManager_;

	bool running_;
	
	int width_, height_;
	
	SplashScreen *splashScreen_;

	ProjectProperties properties_;
	
	Orientation hardwareOrientation_;
    
    Orientation deviceOrientation_;

	bool luaFilesLoaded_;

#if THREADED_RENDER_LOOP
	NSCondition *renderCond_;
	NSLock *autorotationMutex_;
    CustomThread *renderThread_;
	
	bool renderLoopActive_;
	bool renderTick_;
	
	bool autorotating_;
#endif
	
	int nframe_;
public:
	float scaleFactor_;
    UIView *view_;
};

float gdr_ScaleFactor=1;


NetworkManager::NetworkManager(ApplicationManager* application)
{
	application_ = application;
    server_ = new Server(15000, ::getDeviceName().c_str());
	LuaDebugging::studioLink(server_);
}

NetworkManager::~NetworkManager()
{
	LuaDebugging::studioLink(NULL);
	delete server_;
}

void NetworkManager::tick()
{
	int dataTotal = 0;
	
	while (true)
	{
        if(!openProject_.empty()){
            application_->openProject(openProject_.c_str());
            openProject_.clear();
        }
		int dataSent0 = server_->dataSent();
		int dataReceived0 = server_->dataReceived();
		
		NetworkEvent event;
		server_->tick(&event);
		
		int dataSent1 = server_->dataSent();
		int dataReceived1 = server_->dataReceived();
		
		if (event.eventCode == eDataReceived)
		{
			const std::vector<char>& data = event.data;
            LuaDebugging::studioCommand(data);
			
			switch (data[0])
			{
				case gptMakeDir:
					createFolder(data);
					break;
				case gptWriteFile:
					createFile(data);
					break;
                case gptPlay:{
                    const char* absfilename = g_pathForFile("../luafiles.txt");
                    FILE* fos = fopen(absfilename, "wb");
                    fwrite(&data[0], data.size(), 1, fos);
                    fclose(fos);
					play(data);
                }
					break;
				case gptStop:
					stop();
					break;
				case gptGetFileList:
					sendFileList();
					break;
				case gptSetProjectName:
					setProjectName(data);
					break;
				case gptDeleteFile:
					deleteFile(data);
					break;
                case gptSetProperties:{
                    const char* absfilename = g_pathForFile("../properties.bin");
                    FILE* fos = fopen(absfilename, "wb");
                    fwrite(&data[0], data.size(), 1, fos);
                    fclose(fos);
					setProperties(data);
                }
					break;
			}
		}
		
		int dataDelta = (dataSent1 - dataSent0) + (dataReceived1 - dataReceived0);
		dataTotal += dataDelta;
		
		if (dataDelta == 0 || dataTotal > 1024)
			break;
	}
}

void NetworkManager::createFolder(const std::vector<char>& data)
{
	std::string folderName = &data[1];
	mkdir(g_pathForFile(folderName.c_str()), 0755);
}

void NetworkManager::createFile(const std::vector<char>& data)
{
	std::string fileName = &data[1];
	const char* absfilename = g_pathForFile(fileName.c_str());
	FILE* fos = fopen(absfilename, "wb");
	int pos = 1 + fileName.size() + 1;
	if (data.size() > pos)
		fwrite(&data[pos], data.size() - pos, 1, fos);
	fclose(fos);
	calculateMD5(fileName.c_str());
	saveMD5();
}

void NetworkManager::play(const std::vector<char> &data)
{
	std::vector<std::string> luafiles;
	
	ByteBuffer buffer(&data[0], data.size());
	
	char chr;
	buffer >> chr;
	
	while (buffer.eob() == false)
	{
		std::string str;
		buffer >> str;
		luafiles.push_back(str);
	}
	
	application_->play(luafiles);
}

void NetworkManager::stop()
{
	application_->stop();
}

void NetworkManager::sendFileList()
{
	ByteBuffer buffer;
	
	// type(byte) 6
	// D or F, file (zero ended string), age (int)
	// D or F, file (zero ended string), age (int)
	// ....
	
	buffer.append((char)6);
	
	std::vector<std::string> files, directories;
	getDirectoryListingR(resourceDirectory_.c_str(), &files, &directories);
	
	for (std::size_t i = 0; i < files.size(); ++i)
	{
		buffer.append('F');
		buffer.append(files[i]);
		int age = fileAge(pathForFileEx(resourceDirectory_.c_str(), files[i].c_str()));
		buffer.append(age);
		
		std::map<std::string, std::vector<unsigned char> >::iterator iter = md5_.find(files[i]);
		if (iter == md5_.end())
		{
			calculateMD5(files[i].c_str());
			saveMD5();
			iter = md5_.find(files[i]);
		}
		buffer.append(&iter->second[0], 16);
	}
	
	for (std::size_t i = 0; i < directories.size(); ++i)
	{
		buffer.append('D');
		buffer.append(directories[i]);
	}
	
	server_->sendData(buffer.data(), buffer.size());
}

void NetworkManager::setProjectName(const std::vector<char> &data)
{
	ByteBuffer buffer(&data[0], data.size());
	
	char chr;
	buffer >> chr;
	
	std::string str;
	buffer >> str;
	
	application_->setProjectName(str.c_str());
}

void NetworkManager::deleteFile(const std::vector<char> &data)
{
	ByteBuffer buffer(&data[0], data.size());
	
	char chr;
	buffer >> chr;
	
	std::string fileName;
	buffer >> fileName;
	
	remove(g_pathForFile(fileName.c_str()));
	
	{
		std::map<std::string, std::vector<unsigned char> >::iterator iter = md5_.find(fileName);
		if (iter != md5_.end())
		{
			md5_.erase(iter);
			saveMD5();
		}
	}
}

void NetworkManager::setProperties(const std::vector<char> &data)
{
	ProjectProperties properties;
	properties.load(data, true);
    if (properties.windowWidth == 0 && properties.windowHeight == 0) {
        properties.windowWidth = properties.logicalWidth;
        properties.windowHeight = properties.logicalHeight;
    }

	application_->setProjectProperties(properties);
}

void NetworkManager::loadMD5()
{
	md5_.clear();
	
	FILE* fis = fopen(md5filename_.c_str(), "rb");
	
	if (fis == NULL)
		return;
	
	int nfiles;
	fread(&nfiles, sizeof(int), 1, fis);
	
	for (int i = 0; i < nfiles; ++i)
	{
		int strlen;
		fread(&strlen, sizeof(int), 1, fis);
		
		char* buffer = (char*)malloc(strlen);
		fread(buffer, 1, strlen, fis);
		std::string filename(buffer, strlen);
		free(buffer);
		
		std::vector<unsigned char> md5(16);
		fread(&md5[0], 1, 16, fis);
		
		md5_[filename] = md5;
	}
}
void NetworkManager::saveMD5()
{
	FILE* fos = fopen(md5filename_.c_str(), "wb");
	if (fos == NULL)
		return;
	
	int nfiles = md5_.size();
	fwrite(&nfiles, sizeof(int), 1, fos);
	
	int i = 0;
	std::map<std::string, std::vector<unsigned char> >::iterator iter, end = md5_.end();
	for (iter = md5_.begin(); iter != end; ++iter, ++i)
	{
		int strlen = iter->first.size();
		fwrite(&strlen, sizeof(int), 1, fos);
		fwrite(iter->first.c_str(), 1, strlen, fos);
		fwrite(&iter->second[0], 1, 16, fos);
	}
	
	fclose(fos);
}
void NetworkManager::calculateMD5(const char* file)
{
	std::vector<unsigned char> md5(16);
	if (md5_fromfile(g_pathForFile(file), &md5[0]))
		md5_[file] = md5;
}

extern void eventFlush();

extern "C" {
UIViewController *g_getRootViewController();
}

static ApplicationManager *s_manager = NULL;

void getSafeDisplayArea(int &l,int &t,int &r,int &b,ApplicationManager *app)
{
		CGRect sa;
        [app->view_ getSafeArea:&sa];
		float scale=app->scaleFactor_;
		l=sa.origin.x*scale;
		t=sa.origin.y*scale;
		r=sa.size.width*scale;
		b=sa.size.height*scale;
}

void getSafeDisplayArea(int &l,int &t,int &r,int &b)
{
    if (s_manager)
        getSafeDisplayArea(l,t,r,b,s_manager);
}

ApplicationManager::ApplicationManager(UIView *view, int width, int height, bool player)
{
	view_ = view;
	width_ = width;
	height_ = height;
	player_ = player;
    keyboardVisible_=false;

	// gpath & gvfs
	gpath_init();
    gpath_addDrivePrefix(0, "|R|");
    gpath_addDrivePrefix(0, "|r|");
    gpath_addDrivePrefix(1, "|D|");
    gpath_addDrivePrefix(1, "|d|");
    gpath_addDrivePrefix(2, "|T|");
    gpath_addDrivePrefix(2, "|t|");
	
    gpath_setDriveFlags(0, GPATH_RO | GPATH_REAL);
    gpath_setDriveFlags(1, GPATH_RW | GPATH_REAL);
    gpath_setDriveFlags(2, GPATH_RW | GPATH_REAL);
	
	gpath_setAbsolutePathFlags(GPATH_RW | GPATH_REAL);
	
    gpath_setDefaultDrive(0);

	gvfs_init();

	// event
	gevent_Init();
	gevent_SetFlusher(eventFlush);

    // application
    gapplication_init();
	
	// input
    ginput_init();
	
	// geolocation
#if TARGET_OS_TV == 0
	ggeolocation_init();
#endif

	// http
	ghttp_Init();
    
	// ui
	gui_init();
    
    // texture
    gtexture_init();

	// audio
    gaudio_Init();

	// network
	if (player_)
		networkManager_ = new NetworkManager(this);
	else
		networkManager_ = NULL;
	
	// application
	application_ = new LuaApplication;
    application_->setPlayerMode(player_);
	if (player_)
		application_->setPrintFunc(NetworkManager::printToServer_s, networkManager_);
	application_->enableExceptions();
	application_->initialize();
    application_->setResolution(width_, height_);
    hardwareOrientation_ = ePortrait;
    #if TARGET_OS_OSX
    hardwareOrientation_ = eFixed;
    #endif
    deviceOrientation_ = ePortrait;

    #if TARGET_OS_TV == 1
    hardwareOrientation_ = eLandscapeLeft;
    deviceOrientation_ = eLandscapeLeft;
    #endif

#if !TARGET_OS_TV && !TARGET_OS_OSX
    willRotateToInterfaceOrientationHelper([UIApplication sharedApplication].statusBarOrientation);
#else
    willRotateToInterfaceOrientationHelperTV(eLandscapeRight);
#endif
    scaleFactor_=1;
	int sl,st,sr,sb;
	getSafeDisplayArea(sl,st,sr,sb,this);
	drawInfoMargins(sl,st);

	Binder::disableTypeChecking();
	
	running_ = false;

#if THREADED_RENDER_LOOP
	autorotationMutex_ = [[NSLock alloc] init];
    renderCond_ = [[NSCondition alloc] init];
	autorotating_ = false;
#endif
	
	nframe_ = 0;
	
	splashScreen_ = NULL;

	if (player_ == false)
	{
		NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
		NSString* documentsDirectory = [paths objectAtIndex:0];
		printf("%s\n", [documentsDirectory UTF8String]);
		
		NSString* temporaryDirectory = NSTemporaryDirectory();
		printf("%s\n", [temporaryDirectory UTF8String]);
		
		NSString* resourceDirectory = [[NSBundle mainBundle] resourcePath];
		printf("%s\n", [resourceDirectory UTF8String]);
        
        NSArray *paths2 = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
        NSString *cachesDirectory = [paths2 objectAtIndex:0];
        printf("%s\n", [cachesDirectory UTF8String]);
		
#if TARGET_OS_TV == 1
		setDocumentsDirectory([cachesDirectory UTF8String]);
		setTemporaryDirectory([temporaryDirectory UTF8String]);
		setResourceDirectory(pathForFileEx([resourceDirectory UTF8String], "assets"));	
#else
        setDocumentsDirectory([documentsDirectory UTF8String]);
		setTemporaryDirectory([temporaryDirectory UTF8String]);
		setResourceDirectory(pathForFileEx([resourceDirectory UTF8String], "assets"));
#endif
		loadProperties();

		// Gideros has became open source and free, because this, there's no more splash art
		loadLuaFiles();

		/*
		bool licensed = (licenseKey_[15] == 'f' && licenseKey_[16] == 'f');

		if (licensed)
		{
			loadLuaFiles();
		}
		else
		{
			application_->getApplication()->setBackgroundColor(0, 0, 0);
			splashScreen_ = new SplashScreen(application_->getApplication());
			application_->getApplication()->stage()->addChild(splashScreen_);
			splashScreen_->unref();
		}		
		*/
	}
}

ApplicationManager::~ApplicationManager()
{	
	// application
	application_->deinitialize();
	delete application_;

	// network
	if (networkManager_)
		delete networkManager_;

	// audio
    gaudio_Cleanup();

    // texture
    gtexture_cleanup();
	
    // ui
	gui_cleanup();
	
	// http
	ghttp_Cleanup();
	
	// geolocation
#if TARGET_OS_TV == 0
	ggeolocation_cleanup();
#endif
	
	// input
    ginput_cleanup();
	
    // application
    gapplication_cleanup();
    
	// event
	gevent_Cleanup();

	// gpath & gvfs
	gvfs_cleanup();

	gpath_cleanup();
    
    [renderCond_ release];
    [autorotationMutex_ release];
}

void ApplicationManager::drawFirstFrame()
{
#if !TARGET_OS_TV && !TARGET_OS_OSX
    willRotateToInterfaceOrientationHelper([UIApplication sharedApplication].statusBarOrientation);
#else
    willRotateToInterfaceOrientationHelperTV(eLandscapeRight);
#endif

	[view_ setFramebuffer];
	application_->clearBuffers();
	application_->renderScene(1);
	drawIPs();
	[view_ presentFramebuffer];
}

void ApplicationManager::setOpenProject(const char* project){
    networkManager_->openProject_ = project;
}

void ApplicationManager::openProject(const char* project){
    
    //setting project name
    setProjectName(project);
    
    //setting properties
    const char* propfilename = g_pathForFile("../properties.bin");
    FILE* fis_prop = fopen(propfilename, "rb");
    
    const char* luafilename = g_pathForFile("../luafiles.txt");
    FILE* fis_lua = fopen(luafilename, "rb");
    
    if(fis_prop != NULL && fis_lua != NULL){
        
        fseek(fis_prop, 0, SEEK_END);
        int len = ftell(fis_prop);
        fseek(fis_prop, 0, SEEK_SET);
        
        std::vector<char> buf_prop(len);
        fread(&buf_prop[0], 1, len, fis_prop);
        fclose(fis_prop);
        
        ProjectProperties properties;
        
		properties.load(buf_prop, true);
        if (properties.windowWidth == 0 && properties.windowHeight == 0) {
            properties.windowWidth = properties.logicalWidth;
            properties.windowHeight = properties.logicalHeight;
        }

        setProjectProperties(properties);
        
        //loading lua files
        std::vector<std::string> luafiles;
        
        const char* luafilename = g_pathForFile("../luafiles.txt");
        FILE* fis_lua = fopen(luafilename, "rb");
        
        fseek(fis_lua, 0, SEEK_END);
        len = ftell(fis_lua);
        fseek(fis_lua, 0, SEEK_SET);
        
        std::vector<char> buf_lua(len);
        fread(&buf_lua[0], 1, len, fis_lua);
        fclose(fis_lua);
        
        ByteBuffer buffer2(&buf_lua[0], buf_lua.size());
		char chr;        
        buffer2 >> chr;
        
        while (buffer2.eob() == false)
        {
            std::string str;
            buffer2 >> str;
            luafiles.push_back(str);
        }
        
        play(luafiles);
    }
}


#if THREADED_RENDER_LOOP
void *ApplicationManager::renderLoop_s(void *args)
{
	return static_cast<ApplicationManager*>(args)->renderLoop();
}

void *ApplicationManager::renderLoop()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	glog_d("[gideros] starting render thread.");

    while (renderLoopActive_)
    {
        [renderCond_ lock];
        while (!renderTick_)
            [renderCond_ wait];

        [autorotationMutex_ lock];

        if (renderLoopActive_)
        {
            @autoreleasepool {                
                [view_ setFramebuffer];
                application_->clearBuffers();
                application_->renderScene(1);
                drawIPs();
                [view_ presentFramebuffer];
            }
        }
		
        [autorotationMutex_ unlock];

		renderTick_ = false;
        [renderCond_ unlock];
    }	
	
	
	glog_d("[gideros] ending render thread.");
	
	[pool release];
	
	return NULL;
}

#endif

void ApplicationManager::drawFrame(bool force)
{
#if THREADED_RENDER_LOOP
	if (autorotating_)
		return;
#endif
	
	nframe_++;
#if THREADED_RENDER_LOOP
    [renderCond_ lock];
#endif
	
	if (networkManager_)
		networkManager_->tick();

	if (splashScreen_ && splashScreen_->isFinished())
	{
		application_->getApplication()->stage()->removeChild(splashScreen_);
		splashScreen_ = NULL;
		application_->getApplication()->setBackgroundColor(1, 1, 1);
		loadLuaFiles();
	
		[view_ setFramebuffer];
		application_->clearBuffers();
		application_->renderScene(1);
		drawIPs();
		[view_ presentFramebuffer]; 

#if THREADED_RENDER_LOOP
        [renderCond_ unlock];
#endif
		
		return;
	}

#if !THREADED_RENDER_LOOP
    [view_ setFramebuffer];
    application_->clearBuffers();
#endif

	if (application_->isErrorSet())
		luaError(application_->getError());

    GStatus status;
    application_->enterFrame(&status);
    if (status.error())
        luaError(status.errorString());

    bool now=force;
    if (!application_->onDemandDraw(now))
        now=true;
    if (now||force) {
#if THREADED_RENDER_LOOP
        renderTick_ = true;
        [renderCond_ signal];
#endif
    
#if !THREADED_RENDER_LOOP
        application_->renderScene(1);
        drawIPs();
        [view_ presentFramebuffer];
#endif
    }
#if THREADED_RENDER_LOOP
    [renderCond_ unlock];
#endif
}
	

void ApplicationManager::luaError(const char *error)
{
	glog_e("%s", error);
    
    if (player_ == true)
	{
		running_ = false;
		
		networkManager_->printToServer(error, -1);
		networkManager_->printToServer("\n", -1);
		application_->deinitialize();
		application_->initialize();	
	}
	else
	{
        [view_ reportLuaError:[NSString stringWithUTF8String:error]];
		//g_exit();
	}
}


void ApplicationManager::loadProperties()
{
	G_FILE* fis = g_fopen("properties.bin", "rb");
	
	g_fseek(fis, 0, SEEK_END);
	int len = g_ftell(fis);
	g_fseek(fis, 0, SEEK_SET);
	
	std::vector<char> buf(len);
	g_fread(&buf[0], 1, len, fis);
	g_fclose(fis);
	
	properties_.load(buf, false);
    if (properties_.windowWidth == 0 && properties_.windowHeight == 0) {
        properties_.windowWidth = properties_.logicalWidth;
        properties_.windowHeight = properties_.logicalHeight;
    }

#if !TARGET_OS_OSX
	bool phone = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone;
#else
    bool phone= false;
#endif
	bool notRetina = (properties_.retinaDisplay == 0) || (properties_.retinaDisplay == 1 && !phone) || (properties_.retinaDisplay == 2 && phone);
	
	float contentScaleFactor = 1;
#if !TARGET_OS_OSX
    [view_ enableRetinaDisplay:(notRetina ? NO : YES)];
    if ([view_ respondsToSelector:@selector(contentScaleFactor)] == YES)
        contentScaleFactor = view_.contentScaleFactor;
#else
    [view_ enableRetinaDisplay:(notRetina ? NO : YES) scalePtr:&contentScaleFactor];
#endif
	scaleFactor_=contentScaleFactor;
    gdr_ScaleFactor=contentScaleFactor;
    application_->setResolution(width_ * contentScaleFactor, height_ * contentScaleFactor);
	application_->setHardwareOrientation(hardwareOrientation_);
	application_->getApplication()->setDeviceOrientation(deviceOrientation_);
	application_->setOrientation((Orientation)properties_.orientation);
    application_->setLogicalDimensions(properties_.logicalWidth, properties_.logicalHeight);
	application_->setLogicalScaleMode((LogicalScaleMode)properties_.scaleMode);
	application_->setImageScales(properties_.imageScales);
#if TARGET_OS_OSX
    if (properties_.windowWidth != 0 && properties_.windowHeight != 0) {
        if ((properties_.orientation==eLandscapeLeft)||
            (properties_.orientation==eLandscapeRight))
            setWindowSize(properties_.windowHeight,properties_.windowWidth);
        else
            setWindowSize(properties_.windowWidth,properties_.windowHeight);
    }
#endif
#if !TARGET_OS_TV && !TARGET_OS_OSX
    willRotateToInterfaceOrientationHelper([UIApplication sharedApplication].statusBarOrientation);
#else
    willRotateToInterfaceOrientationHelperTV(eLandscapeRight);
#endif

	g_setFps(properties_.fps);
	ginput_setMouseToTouchEnabled(properties_.mouseToTouch);
	ginput_setTouchToMouseEnabled(properties_.touchToMouse);
	ginput_setMouseTouchOrder(properties_.mouseTouchOrder);
}

void ApplicationManager::loadLuaFiles()
{
	std::vector<std::string> luafiles;
	
	G_FILE* fis = g_fopen("luafiles.txt", "rt");
	
	if (fis)
	{
		char line[1024];
		while (true)
		{
			if (g_fgets(line, 1024, fis) == NULL)
				break;
			
			size_t len = strlen(line);
			
			if (len > 0 && line[len - 1] == 0xa)
				line[--len] = 0;
			
			if (len > 0 && line[len - 1] == 0xd)
				line[--len] = 0;
			
			if (len > 0)
				luafiles.push_back(line);
		}
		
		g_fclose(fis);
	}
	
    GStatus status;
    for (size_t i = 0; i < luafiles.size(); ++i)
    {
        application_->loadFile(luafiles[i].c_str(), &status);
        if (status.error())
            break;
    }

    if (!status.error())
    {
        gapplication_enqueueEvent(GAPPLICATION_START_EVENT, NULL, 0);
        application_->tick(&status);
    }
        
    if (status.error())
        luaError(status.errorString());
}

void ApplicationManager::play(const std::vector<std::string>& luafiles)
{
    running_ = true;
	
	application_->deinitialize();
	application_->initialize();

#if !TARGET_OS_OSX
	bool phone = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone;
#else
    bool phone = false;
#endif
	bool notRetina = (properties_.retinaDisplay == 0) || (properties_.retinaDisplay == 1 && !phone) || (properties_.retinaDisplay == 2 && phone);
	
	float contentScaleFactor = 1;
#if !TARGET_OS_OSX
    [view_ enableRetinaDisplay:(notRetina ? NO : YES)];
	if ([view_ respondsToSelector:@selector(contentScaleFactor)] == YES)
		contentScaleFactor = view_.contentScaleFactor;
#else
    [view_ enableRetinaDisplay:(notRetina ? NO : YES) scalePtr:&contentScaleFactor];
#endif
	scaleFactor_=contentScaleFactor;
    gdr_ScaleFactor=contentScaleFactor;
	application_->setResolution(width_ * contentScaleFactor, height_ * contentScaleFactor);
	application_->setHardwareOrientation(hardwareOrientation_);
	application_->getApplication()->setDeviceOrientation(deviceOrientation_);
	application_->setOrientation((Orientation)properties_.orientation);
	application_->setLogicalDimensions(properties_.logicalWidth, properties_.logicalHeight);
	application_->setLogicalScaleMode((LogicalScaleMode)properties_.scaleMode);
	application_->setImageScales(properties_.imageScales);
#if TARGET_OS_OSX
    if (properties_.windowWidth != 0 && properties_.windowHeight != 0) {
        if ((properties_.orientation==eLandscapeLeft)||
            (properties_.orientation==eLandscapeRight))
            setWindowSize(properties_.windowHeight,properties_.windowWidth);
        else
            setWindowSize(properties_.windowWidth,properties_.windowHeight);
    }
#endif
    
#if !TARGET_OS_TV && !TARGET_OS_OSX
    willRotateToInterfaceOrientationHelper([UIApplication sharedApplication].statusBarOrientation);
#else
    willRotateToInterfaceOrientationHelperTV(eLandscapeRight);
#endif

	g_setFps(properties_.fps);
	ginput_setMouseToTouchEnabled(properties_.mouseToTouch);
	ginput_setTouchToMouseEnabled(properties_.touchToMouse);
	ginput_setMouseTouchOrder(properties_.mouseTouchOrder);

    GStatus status;
    for (std::size_t i = 0; i < luafiles.size(); ++i)
    {
        application_->loadFile(luafiles[i].c_str(), &status);
        if (status.error())
            break;
    }

    if (!status.error())
    {
        gapplication_enqueueEvent(GAPPLICATION_START_EVENT, NULL, 0);
        application_->tick(&status);
    }

    if (status.error())
		luaError(status.errorString());
}

void ApplicationManager::stop()
{
    if (running_ == true)
	{
        gapplication_enqueueEvent(GAPPLICATION_EXIT_EVENT, NULL, 0);

        GStatus status;
        application_->tick(&status);
        if (status.error())
            luaError(status.errorString());
	}
	
	running_ = false;
	
	application_->deinitialize();
	application_->initialize();
}

bool ApplicationManager::isRunning()
{
    return running_;
}

void ApplicationManager::drawIPs()
{
	if (player_ == true && running_ == false)
	{	
		drawInfo();
	}
}

void ApplicationManager::setProjectName(const char *projectName)
{
	glog_v("setProjectName: %s", projectName);
    NSArray* paths;
#if TARGET_OS_TV == 1
    paths = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
#else
    paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
#endif
    
	std::string dir = [[paths objectAtIndex:0] UTF8String];
	
	if (dir[dir.size() - 1] != '/')
		dir += "/";
	
	dir += "gideros";
	
	mkdir(dir.c_str(), 0755);
	
	dir += "/";
	
	dir += projectName;
	
	mkdir(dir.c_str(), 0755);
	
	dir += "/";
	
	std::string md5filename_ = dir + "md5.txt";
	
	std::string documents = dir + "documents";
	std::string temporary = dir + "temporary";
	std::string resource = dir + "resource";
	
	glog_v("documents: %s", documents.c_str());
	glog_v("temporary: %s", temporary.c_str());
	glog_v("resource: %s", resource.c_str());
	
	mkdir(documents.c_str(), 0755);
	mkdir(temporary.c_str(), 0755);
	mkdir(resource.c_str(), 0755);
	
	setDocumentsDirectory(documents.c_str());
	setTemporaryDirectory(temporary.c_str());
	setResourceDirectory(resource.c_str());
	
	std::string resourceDirectory_ = resource;
	
	networkManager_->setResourceDirectory(resourceDirectory_.c_str());
	networkManager_->setMd5FileName(md5filename_.c_str());
}


void ApplicationManager::setProjectProperties(const ProjectProperties &properties)
{
	properties_ = properties;
}

void ApplicationManager::forceTick()
{
 	if (!renderLoopActive_)
 	{
    	GStatus status;
    	application_->tick(&status);
    	if (status.error())
        	luaError(status.errorString());
    }
}

void ApplicationManager::suspend()
{
    gapplication_enqueueEvent(GAPPLICATION_PAUSE_EVENT, NULL, 0);
#if THREADED_RENDER_LOOP
    [renderCond_ lock];
#endif
    GStatus status;
    application_->tick(&status);
    if (status.error())
        luaError(status.errorString());
    if (networkManager_)
    {
        delete networkManager_;
        networkManager_=NULL;
        if (player_)
            application_->setPrintFunc(NetworkManager::printToServer_s, NULL);
    }

#if THREADED_RENDER_LOOP
    [renderCond_ unlock];
#endif
    
    exitRenderLoopHelper();
}

void ApplicationManager::resume()
{
    gapplication_enqueueEvent(GAPPLICATION_RESUME_EVENT, NULL, 0);

#if THREADED_RENDER_LOOP
    [renderCond_ lock];
#endif
    if (player_&&!networkManager_)
    {
        networkManager_ = new NetworkManager(this);
        application_->setPrintFunc(NetworkManager::printToServer_s, networkManager_);
    }

    GStatus status;
    application_->tick(&status);
    if (status.error())
        luaError(status.errorString());
#if THREADED_RENDER_LOOP
    [renderCond_ unlock];
#endif
    
#if THREADED_RENDER_LOOP
    renderLoopActive_ = true;
	renderTick_ = false;
    renderThread_ = [[CustomThread alloc] initWithStartRoutine:renderLoop_s withArg:this];
    [renderThread_ start];
#endif
}

void ApplicationManager::exitRenderLoop()
{
	ShaderEngine::setReady(false);
    gapplication_enqueueEvent(GAPPLICATION_EXIT_EVENT, NULL, 0);
    
#if THREADED_RENDER_LOOP
    [renderCond_ lock];
#endif
    GStatus status;
    application_->tick(&status);
    if (status.error())
        luaError(status.errorString());
#if THREADED_RENDER_LOOP
    [renderCond_ unlock];
#endif
    
    exitRenderLoopHelper();
}

void ApplicationManager::exitRenderLoopHelper()
{
#if THREADED_RENDER_LOOP
    if (renderThread_ == nil)
        return;
    [renderCond_ lock];
	renderLoopActive_ = false;
	renderTick_ = true;
    [renderCond_ signal];
    [renderCond_ unlock];
    
    while (![renderThread_ isFinished])
        [NSThread sleepForTimeInterval:0.01];
    [renderThread_ release];
    renderThread_ = nil;
#endif
}

#if TARGET_OS_TV == 0
void ApplicationManager::touchesBegan(NSSet *touches, NSSet *allTouches)
{
    ginputp_touchesBegan(touches, allTouches, (UIView*)view_);
}

void ApplicationManager::touchesMoved(NSSet *touches, NSSet *allTouches)
{
    ginputp_touchesMoved(touches, allTouches, (UIView*)view_);
}

void ApplicationManager::touchesEnded(NSSet *touches, NSSet *allTouches)
{
    ginputp_touchesEnded(touches, allTouches, (UIView*)view_);
}

void ApplicationManager::touchesCancelled(NSSet *touches, NSSet *allTouches)
{
    ginputp_touchesCancelled(touches, allTouches, (UIView*)view_);
}

void ApplicationManager::touchesReset()
{
    ginputp_touchesReset((UIView*)view_);
}
#endif

void ApplicationManager::keyDown(int keyCode, int mods, int repeat)
{
    ginputp_keyDown(keyCode,mods,repeat);
}

void ApplicationManager::keyUp(int keyCode, int mods, int repeat)
{
    ginputp_keyUp(keyCode,mods,repeat);
}

void ApplicationManager::keyChar(NSString *text)
{
    ginputp_keyChar([text UTF8String]);
}


void ApplicationManager::didReceiveMemoryWarning()
{
    gapplication_enqueueEvent(GAPPLICATION_MEMORY_LOW_EVENT, NULL, 0);
    
#if THREADED_RENDER_LOOP
    [renderCond_ lock];
#endif
    GStatus status;
    application_->tick(&status);
    if (status.error())
        luaError(status.errorString());
#if THREADED_RENDER_LOOP
    [renderCond_ unlock];
#endif
}

#if !TARGET_OS_TV && !TARGET_OS_OSX
BOOL ApplicationManager::shouldAutorotateToInterfaceOrientation(UIInterfaceOrientation interfaceOrientation)
{
	return (properties_.autorotation==1)||(properties_.autorotation==2);
}

NSUInteger ApplicationManager::supportedInterfaceOrientations()
{
	NSUInteger result=UIInterfaceOrientationMaskPortrait;
	
	if (properties_.autorotation==1) {
		switch (application_->orientation()) {
			case eLandscapeLeft:
			case eLandscapeRight:
				result=UIInterfaceOrientationMaskLandscapeLeft|UIInterfaceOrientationMaskLandscapeRight;
                break;
			case ePortrait:
			case ePortraitUpsideDown:
				result=UIInterfaceOrientationMaskPortrait|UIInterfaceOrientationMaskPortraitUpsideDown;
                break;
		}
	}
	else if (properties_.autorotation==2) {
        result= UIInterfaceOrientationMaskAll;
	}
	else {
		switch (application_->orientation()) {
			case eLandscapeLeft:
				result=UIInterfaceOrientationMaskLandscapeLeft;
                break;
			case eLandscapeRight:
				result=UIInterfaceOrientationMaskLandscapeRight;
                break;
			case ePortrait:
				result=UIInterfaceOrientationMaskPortrait;
                break;
			case ePortraitUpsideDown:
				result=UIInterfaceOrientationMaskPortraitUpsideDown;
                break;
		}
    }

    return result;
}
#endif


#if TARGET_OS_TV || TARGET_OS_OSX
void ApplicationManager::willRotateToInterfaceOrientationHelperTV(Orientation deviceOrientation_)
{
    application_->getApplication()->setDeviceOrientation(deviceOrientation_);
    application_->setHardwareOrientation(hardwareOrientation_);

}
#else
void ApplicationManager::willRotateToInterfaceOrientationHelper(UIInterfaceOrientation toInterfaceOrientation)
{
    switch (toInterfaceOrientation)
    {
        case UIInterfaceOrientationPortrait:
            deviceOrientation_ = ePortrait;
            break;
        case UIInterfaceOrientationPortraitUpsideDown:
            deviceOrientation_ = ePortraitUpsideDown;
            break;
        case UIInterfaceOrientationLandscapeRight:
            deviceOrientation_ = eLandscapeRight;
            break;
        case UIInterfaceOrientationLandscapeLeft:
            deviceOrientation_ = eLandscapeLeft;
            break;
        default:
            deviceOrientation_ = ePortrait;
            break;
    }
    application_->getApplication()->setDeviceOrientation(deviceOrientation_);

    hardwareOrientation_ = deviceOrientation_;
    application_->setHardwareOrientation(hardwareOrientation_);
}

void ApplicationManager::willRotateToInterfaceOrientation(UIInterfaceOrientation toInterfaceOrientation)
{
#if THREADED_RENDER_LOOP
    [autorotationMutex_ lock];
	autorotating_ = true;
#endif
    
    willRotateToInterfaceOrientationHelper(toInterfaceOrientation);
}

void ApplicationManager::didRotateFromInterfaceOrientation(UIInterfaceOrientation fromInterfaceOrientation)
{
#if THREADED_RENDER_LOOP
	autorotating_ = false;
    [autorotationMutex_ unlock];
#endif
}
#endif

void ApplicationManager::requestDeviceOrientation(gapplication_Orientation iO,gapplication_AutoOrientation iAutoRot)
{
#if TARGET_OS_TV || TARGET_OS_OSX
#else
	properties_.autorotation=iAutoRot;
	UIInterfaceOrientation ior=UIInterfaceOrientationPortrait;
	switch (iO) {
		case GAPPLICATION_LANDSCAPE_LEFT: ior=UIInterfaceOrientationLandscapeLeft; break;
		case GAPPLICATION_LANDSCAPE_RIGHT: ior=UIInterfaceOrientationLandscapeRight; break;
		case GAPPLICATION_PORTRAIT_UPSIDE_DOWN: ior=UIInterfaceOrientationPortraitUpsideDown; break;
	}
    if (@available (iOS 16, iPadOS 16, tvOS 16, *)) {
        [UIViewController setNeedsUpdateOfSupportedInterfaceOrientations];
    }
    else {
        NSNumber *value = [NSNumber numberWithInt:ior];
        [[UIDevice currentDevice] setValue:value forKey:@"orientation"];
        [UIViewController attemptRotationToDeviceOrientation];
    }
#endif
}
 
void ApplicationManager::handleOpenUrl(NSURL *url)
{
    gapplication_OpenUrlEvent *event = (gapplication_OpenUrlEvent*)gevent_CreateEventStruct1(
                                           sizeof(gapplication_OpenUrlEvent),
                                        offsetof(gapplication_OpenUrlEvent, url), [[url absoluteString] UTF8String]);

    gapplication_enqueueEvent(GAPPLICATION_OPEN_URL_EVENT, event, 1);
}

void ApplicationManager::foreground()
{
    gapplication_enqueueEvent(GAPPLICATION_FOREGROUND_EVENT, NULL, 0);

#if THREADED_RENDER_LOOP
    [renderCond_ lock];
#endif
    GStatus status;
    application_->tick(&status);
    if (status.error())
        luaError(status.errorString());
#if THREADED_RENDER_LOOP
    [renderCond_ unlock];
#endif
}

void ApplicationManager::background()
{
    gapplication_enqueueEvent(GAPPLICATION_BACKGROUND_EVENT, NULL, 0);

#if THREADED_RENDER_LOOP
    [renderCond_ lock];
#endif
    GStatus status;
    application_->tick(&status);
    if (status.error())
        luaError(status.errorString());
#if THREADED_RENDER_LOOP
    [renderCond_ unlock];
#endif
}

void ApplicationManager::surfaceChanged(int width,int height)
{
#if TARGET_OS_OSX
    width_ = width;
    height_ = height;
    application_->setResolution(width_, height_);
#endif
	int sl,st,sr,sb;
	getSafeDisplayArea(sl,st,sr,sb,this);
	drawInfoMargins(sl,st);
    if (ShaderEngine::Engine)
    	ShaderEngine::Engine->resizeFramebuffer(width, height);
    Event event(Event::APPLICATION_RESIZE);
    GStatus status;
    application_->broadcastEvent(&event, &status);
    if (status.error())
        luaError(status.errorString());
}


bool ApplicationManager::isKeyboardVisible(){
    return keyboardVisible_;
}

bool ApplicationManager::setKeyboardVisibility(bool visible)
{
    keyboardVisible_=visible;
    if (visible)
        [view_ becomeFirstResponder];
    else
        [view_ resignFirstResponder];
#if TARGET_OS_OSX
    return false; //No virtual keyboard
#else
    return true;
#endif    
}

bool setKeyboardVisibility(bool visible){
    if (s_manager)
        return s_manager->setKeyboardVisibility(visible);
    return false;
}

bool ApplicationManager::setTextInput(int type,const char *buffer,int selstart,int selend,const char *label,const char *actionLabel, const char *hintText, const char *context)
{
#if TARGET_OS_OSX
    return false; //No virtual keyboard
#else
	UIKeyboardType kt=UIKeyboardTypeDefault;
	switch (type) {
		case 0: //None
		break;
		case 1: //Text
			if (type&0x10) //URI
				kt=UIKeyboardTypeURL;
			if (type&0x20) //Email
				kt=UIKeyboardTypeEmailAddress;
		break;
		case 2: //Number
			kt=UIKeyboardTypeNumberPad;
			if (type&0x2000) //Decimal
				kt=UIKeyboardTypeDecimalPad;
			if (type&0x1000) //Signed
				kt=UIKeyboardTypeNumbersAndPunctuation;
		break;
		case 3: //Phone
			kt=UIKeyboardTypePhonePad;
		break;
		case 4: //Date
			kt=UIKeyboardTypeNumbersAndPunctuation;
		break;
	}
	view_.keyboardType=kt;
#endif    
	return false;
}

bool setTextInput(int type,const char *buffer,int selstart,int selend,const char *label,const char *actionLabel, const char *hintText, const char *context)
{
    if (s_manager)
        return s_manager->setTextInput(type,buffer,selstart,selend,label,actionLabel,hintText,context);
    return false;
}

int setClipboard(std::string data,std::string mimeType, int luaFunc) {
#if TARGET_OS_OSX
    NSPasteboard *pb=[NSPasteboard generalPasteboard];
    if (mimeType=="text/plain") {
        [pb clearContents];
        [pb declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString] owner:nil];
        [pb setString:[NSString stringWithUTF8String:data.c_str()] forType:NSPasteboardTypeString];
        return 1;
    }
#elif !TARGET_OS_TV
    UIPasteboard *pb=[UIPasteboard generalPasteboard];
    if (mimeType=="text/plain") {
        pb.string=[NSString stringWithUTF8String:data.c_str()];
        return 1;
    }
#endif
	return -1;
}

int getClipboard(std::string &data,std::string &mimeType, int luaFunc) {
#if TARGET_OS_OSX
    NSString *s=[[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
    if (s!=nil) {
        data=[s UTF8String];
        mimeType="text/plain";
        return 1;
    }
#elif !TARGET_OS_TV
    NSString *s=[UIPasteboard generalPasteboard].string;
    if (s!=nil) {
        data=[s UTF8String];
        mimeType="text/plain";
        return 1;
    }
#endif
    return -1;
}

int getKeyboardModifiers() {
	return 0;
}

void eventFlush()
{
    if (s_manager)
        s_manager->forceTick();
}

extern "C" {

void gdr_initialize(UIView* view, int width, int height, bool player)
{
	s_manager = new ApplicationManager(view, width, height, player);
}
    
void gdr_surfaceChanged(int width,int height)
{
    if (s_manager)
         s_manager->surfaceChanged(width,height);
}

void gdr_drawFrame(bool force)
{
	s_manager->drawFrame(force);
}

void gdr_exitGameLoop()
{
	s_manager->exitRenderLoop();
}
    
void gdr_openProject(NSString* project)
{
    
    s_manager->setOpenProject([project UTF8String]);
}


void gdr_deinitialize()
{
	delete s_manager;
	s_manager = NULL;
}

void gdr_drawFirstFrame()
{
    s_manager->drawFirstFrame();        
}
    
void gdr_suspend()
{
	s_manager->suspend();
}

void gdr_resume()
{
	s_manager->resume();
}
#if !TARGET_OS_TV && !TARGET_OS_OSX
BOOL gdr_shouldAutorotateToInterfaceOrientation(UIInterfaceOrientation interfaceOrientation)
{
	return s_manager->shouldAutorotateToInterfaceOrientation(interfaceOrientation);
}

void gdr_willRotateToInterfaceOrientation(UIInterfaceOrientation toInterfaceOrientation)
{
	s_manager->willRotateToInterfaceOrientation(toInterfaceOrientation);
}

void gdr_didRotateFromInterfaceOrientation(UIInterfaceOrientation fromInterfaceOrientation)
{
	s_manager->didRotateFromInterfaceOrientation(fromInterfaceOrientation);
}

NSUInteger gdr_supportedInterfaceOrientations()
{
    return s_manager->supportedInterfaceOrientations();
}
#endif
void gdr_didReceiveMemoryWarning()
{
	s_manager->didReceiveMemoryWarning();
}

#if TARGET_OS_TV == 0
    void gdr_touchesBegan(NSSet *touches, NSSet *allTouches)
    {
        s_manager->touchesBegan(touches, allTouches);
    }
    
    void gdr_touchesMoved(NSSet *touches, NSSet *allTouches)
    {
        s_manager->touchesMoved(touches, allTouches);
    }
    
    void gdr_touchesEnded(NSSet *touches, NSSet *allTouches)
    {
        s_manager->touchesEnded(touches, allTouches);
    }
    
    void gdr_touchesCancelled(NSSet *touches, NSSet *allTouches)
    {
        s_manager->touchesCancelled(touches, allTouches);
    }
    void gdr_touchesReset() {
        s_manager->touchesReset();
    }
#endif
#if TARGET_OS_OSX
    void gdr_mouseDown(int x, int y, int button,int mod) {
        ginputp_mouseDown(x,y,button,mod);
    }
    void gdr_mouseMove(int x, int y, int button,int mod) {
        ginputp_mouseMove(x,y,button,mod);
    }
    void gdr_mouseHover(int x, int y, int button,int mod) {
        ginputp_mouseHover(x,y,button,mod);
    }
    void gdr_mouseUp(int x, int y, int button,int mod) {
        ginputp_mouseUp(x,y,button,mod);
    }
    void gdr_mouseWheel(int x, int y, int button, int delta,int mod) {
        ginputp_mouseWheel(x,y,button,delta,mod);
    }
	void gdr_mouseEnter(int x, int y, int buttons, int mod)
	{
		ginputp_mouseEnter(x, y, buttons, mod);
	}
	void gdr_mouseLeave(int x, int y, int mod)
	{
		ginputp_mouseLeave(x, y, mod);
	}
#endif

void gdr_keyDown(int keyCode, int mods, int repeat)
{
    s_manager->keyDown(keyCode,mods,repeat);
}

void gdr_keyUp(int keyCode, int mods, int repeat)
{
    s_manager->keyUp(keyCode,mods,repeat);
}
    
void gdr_keyChar(NSString *text)
{
    s_manager->keyChar(text);
}


void gdr_handleOpenUrl(NSURL *url)
{
    s_manager->handleOpenUrl(url);
}

void gdr_foreground()
{
    s_manager->foreground();
}

void gdr_background()
{
    s_manager->background();
}
    
BOOL gdr_isRunning()
{
    return s_manager->isRunning();
}
    
BOOL gdr_keyboardVisible()
{
    if (s_manager)
        return s_manager->isKeyboardVisible();
    return FALSE;
}

void gapplication_requestDeviceOrientation(gapplication_Orientation iO,gapplication_AutoOrientation iAutoRot) {
    if (s_manager)
 		s_manager->requestDeviceOrientation(iO,iAutoRot);
}


    
}

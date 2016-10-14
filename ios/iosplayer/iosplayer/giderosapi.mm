#import <UIKit/UIKit.h>

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

#include <splashscreen.h>
#include <application.h>
#include <stage.h>

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

@interface LuaException : NSException
@end
@implementation LuaException
@end


extern "C" {
void g_setFps(int);
int g_getFps();
}
void drawInfo();
void refreshLocalIPs();
void g_exit();

static volatile const char* licenseKey_ = "9852564f4728e0c11e34ca3eb5fe20b2";
//-----------------------------------------01234567890123456------------------

struct ProjectProperties
{
	ProjectProperties()
	{
		scaleMode = 0;
		logicalWidth = 320;
		logicalHeight = 480;
		orientation = 0;
		fps = 60;
		retinaDisplay = 0;
		autorotation = 0;		
		mouseToTouch = 1;
		touchToMouse = 1;
		mouseTouchOrder = 0;
	}

	int scaleMode;
	int logicalWidth;
	int logicalHeight;
	std::vector<std::pair<std::string, float> > imageScales;
	int orientation;
	int fps;
	int retinaDisplay;
	int autorotation;
	int mouseToTouch;
	int touchToMouse;
	int mouseTouchOrder;
};

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
	
//	void surfaceCreated();
//	void surfaceChanged(int width, int height);
	void drawFrame();
	
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

    void keyDown(int keyCode, int repeat);
    void keyUp(int keyCode, int repeat);
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
#if TARGET_OS_TV == 0
	BOOL shouldAutorotateToInterfaceOrientation(UIInterfaceOrientation interfaceOrientation);	
	void willRotateToInterfaceOrientationHelper(UIInterfaceOrientation toInterfaceOrientation);
	void willRotateToInterfaceOrientation(UIInterfaceOrientation toInterfaceOrientation);
	void didRotateFromInterfaceOrientation(UIInterfaceOrientation fromInterfaceOrientation);
    NSUInteger supportedInterfaceOrientations();
#else
        void willRotateToInterfaceOrientationHelperTV(Orientation toInterfaceOrientation);
#endif
    
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
	UIView *view_;
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
};


NetworkManager::NetworkManager(ApplicationManager* application)
{
	application_ = application;
    server_ = new Server(15000, ::getDeviceName().c_str());
}

NetworkManager::~NetworkManager()
{
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
			
			switch (data[0])
			{
				case 0:
					createFolder(data);
					break;
				case 1:
					createFile(data);
					break;
                case 2:{
                    const char* absfilename = g_pathForFile("../luafiles.txt");
                    FILE* fos = fopen(absfilename, "wb");
                    fwrite(&data[0], data.size(), 1, fos);
                    +fclose(fos);
					play(data);
                }
					break;
				case 3:
					stop();
					break;
				case 7:
					sendFileList();
					break;
				case 8:
					setProjectName(data);
					break;
				case 9:
					deleteFile(data);
					break;
                case 11:{
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
	
	ByteBuffer buffer(&data[0], data.size());
	
	char chr;
	buffer >> chr;
	
	buffer >> properties.scaleMode;
	buffer >> properties.logicalWidth;
	buffer >> properties.logicalHeight;
	
	int scaleCount;
	buffer >> scaleCount;
	properties.imageScales.resize(scaleCount);
	for (int i = 0; i < scaleCount; ++i)
	{
		buffer >> properties.imageScales[i].first;
		buffer >> properties.imageScales[i].second;
	}
	
	buffer >> properties.orientation;
	buffer >> properties.fps;
	buffer >> properties.retinaDisplay;
	buffer >> properties.autorotation;
	buffer >> properties.mouseToTouch;
	buffer >> properties.touchToMouse;
	buffer >> properties.mouseTouchOrder;
    
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

    // application
    gapplication_init();
	
	// input
#if TARGET_OS_TV == 0
    ginput_init();
#endif
	
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
#if TARGET_OS_TV == 0
    willRotateToInterfaceOrientationHelper([UIApplication sharedApplication].statusBarOrientation);
#else
    willRotateToInterfaceOrientationHelperTV(eLandscapeRight);
#endif

	Binder::disableTypeChecking();
	
	hardwareOrientation_ = ePortrait;
    
    deviceOrientation_ = ePortrait;

#if TARGET_OS_TV == 1
	hardwareOrientation_ = eLandscapeLeft;
	deviceOrientation_ = eLandscapeLeft;
#endif

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
#if TARGET_OS_TV == 0
    ginput_cleanup();
#endif
	
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
#if TARGET_OS_TV == 0
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
        
        ByteBuffer buffer(&buf_prop[0], buf_prop.size());
        
        char chr;
        buffer >> chr;
        
        buffer >> properties.scaleMode;
        buffer >> properties.logicalWidth;
        buffer >> properties.logicalHeight;
        
        int scaleCount;
        buffer >> scaleCount;
        properties.imageScales.resize(scaleCount);
        for (int i = 0; i < scaleCount; ++i)
        {
            buffer >> properties.imageScales[i].first;
            buffer >> properties.imageScales[i].second;
        }
        
        buffer >> properties.orientation;
        buffer >> properties.fps;
        buffer >> properties.retinaDisplay;
        buffer >> properties.autorotation;
        buffer >> properties.mouseToTouch;
        buffer >> properties.touchToMouse;
        buffer >> properties.mouseTouchOrder;
        
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
            [view_ setFramebuffer];
            application_->clearBuffers();
            application_->renderScene(1);
            drawIPs();
            [view_ presentFramebuffer];
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

void ApplicationManager::drawFrame()
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

#if THREADED_RENDER_LOOP
	renderTick_ = true;
    [renderCond_ signal];
    [renderCond_ unlock];
#endif
    
#if !THREADED_RENDER_LOOP
    application_->renderScene(1);
    drawIPs();
    [view_ presentFramebuffer];
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
        @throw [[LuaException alloc] initWithName:@"Lua" reason:[NSString stringWithUTF8String:error] userInfo:nil];
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
	
	ByteBuffer buffer(&buf[0], buf.size());
	
	buffer >> properties_.scaleMode;
	buffer >> properties_.logicalWidth;
	buffer >> properties_.logicalHeight;
	
	int scaleCount;
	buffer >> scaleCount;
	properties_.imageScales.resize(scaleCount);
	for (int i = 0; i < scaleCount; ++i)
	{
		buffer >> properties_.imageScales[i].first;
		buffer >> properties_.imageScales[i].second;
	}
	
	buffer >> properties_.orientation;
	buffer >> properties_.fps;
	buffer >> properties_.retinaDisplay;
	buffer >> properties_.autorotation;
	buffer >> properties_.mouseToTouch;
	buffer >> properties_.touchToMouse;
	buffer >> properties_.mouseTouchOrder;

	bool phone = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone;
	bool notRetina = (properties_.retinaDisplay == 0) || (properties_.retinaDisplay == 1 && !phone) || (properties_.retinaDisplay == 2 && phone);
	
	float contentScaleFactor = 1;
	[view_ enableRetinaDisplay:(notRetina ? NO : YES)];
	if ([view_ respondsToSelector:@selector(contentScaleFactor)] == YES)
		contentScaleFactor = view_.contentScaleFactor;
	application_->setResolution(width_ * contentScaleFactor, height_ * contentScaleFactor);
	application_->setHardwareOrientation(hardwareOrientation_);
	application_->getApplication()->setDeviceOrientation(deviceOrientation_);
	application_->setOrientation((Orientation)properties_.orientation);
	application_->setLogicalDimensions(properties_.logicalWidth, properties_.logicalHeight);
	application_->setLogicalScaleMode((LogicalScaleMode)properties_.scaleMode);
	application_->setImageScales(properties_.imageScales);
#if TARGET_OS_TV == 0
    willRotateToInterfaceOrientationHelper([UIApplication sharedApplication].statusBarOrientation);
#else
    willRotateToInterfaceOrientationHelperTV(eLandscapeRight);
#endif

	g_setFps(properties_.fps);
#if TARGET_OS_TV == 0
	ginput_setMouseToTouchEnabled(properties_.mouseToTouch);
	ginput_setTouchToMouseEnabled(properties_.touchToMouse);
	ginput_setMouseTouchOrder(properties_.mouseTouchOrder);
#endif
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

	bool phone = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone;
	bool notRetina = (properties_.retinaDisplay == 0) || (properties_.retinaDisplay == 1 && !phone) || (properties_.retinaDisplay == 2 && phone);
	
	float contentScaleFactor = 1;
	[view_ enableRetinaDisplay:(notRetina ? NO : YES)];
	if ([view_ respondsToSelector:@selector(contentScaleFactor)] == YES)
		contentScaleFactor = view_.contentScaleFactor;
	application_->setResolution(width_ * contentScaleFactor, height_ * contentScaleFactor);
	application_->setHardwareOrientation(hardwareOrientation_);
	application_->getApplication()->setDeviceOrientation(deviceOrientation_);
	application_->setOrientation((Orientation)properties_.orientation);
	application_->setLogicalDimensions(properties_.logicalWidth, properties_.logicalHeight);
	application_->setLogicalScaleMode((LogicalScaleMode)properties_.scaleMode);
	application_->setImageScales(properties_.imageScales);
#if TARGET_OS_TV == 0
    willRotateToInterfaceOrientationHelper([UIApplication sharedApplication].statusBarOrientation);
#else
    willRotateToInterfaceOrientationHelperTV(eLandscapeRight);
#endif

	g_setFps(properties_.fps);
#if TARGET_OS_TV == 0
	ginput_setMouseToTouchEnabled(properties_.mouseToTouch);
	ginput_setTouchToMouseEnabled(properties_.touchToMouse);
	ginput_setMouseTouchOrder(properties_.mouseTouchOrder);
#endif

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
#endif

void ApplicationManager::keyDown(int keyCode, int repeat)
{
    ginputp_keyDown(keyCode,repeat);
}

void ApplicationManager::keyUp(int keyCode, int repeat)
{
    ginputp_keyUp(keyCode,repeat);
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

#if TARGET_OS_TV == 0
BOOL ApplicationManager::shouldAutorotateToInterfaceOrientation(UIInterfaceOrientation interfaceOrientation)
{
	BOOL result;
	
	bool phone = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone;
	bool dontAutorotate = (properties_.autorotation == 0) || (properties_.autorotation == 1 && !phone) || (properties_.autorotation == 2 && phone);
	
	if (dontAutorotate)
		result = (interfaceOrientation == UIInterfaceOrientationPortrait);
	else
	{
		if (application_->orientation() == eLandscapeLeft || application_->orientation() == eLandscapeRight)
			result = (interfaceOrientation == UIInterfaceOrientationLandscapeLeft || interfaceOrientation == UIInterfaceOrientationLandscapeRight);
		else
			result = (interfaceOrientation == UIInterfaceOrientationPortrait || interfaceOrientation == UIInterfaceOrientationPortraitUpsideDown);
	}

	return result;
}

NSUInteger ApplicationManager::supportedInterfaceOrientations()
{
	NSUInteger result;
	
	bool phone = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone;
	bool dontAutorotate = (properties_.autorotation == 0) || (properties_.autorotation == 1 && !phone) || (properties_.autorotation == 2 && phone);

    if (dontAutorotate){
        result = UIInterfaceOrientationMaskPortrait;
    }
    else
    {
		if (application_->orientation() == eLandscapeLeft || application_->orientation() == eLandscapeRight)
            result = UIInterfaceOrientationMaskLandscapeLeft | UIInterfaceOrientationMaskLandscapeRight;
        else
            result = UIInterfaceOrientationMaskPortrait | UIInterfaceOrientationMaskPortraitUpsideDown;
    }

    return result;
}
#endif

#if TARGET_OS_TV == 1
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
            deviceOrientation_ = eLandscapeLeft;
            break;
        case UIInterfaceOrientationLandscapeLeft:
            deviceOrientation_ = eLandscapeRight;
            break;
        default:
            deviceOrientation_ = ePortrait;
            break;
    }
    application_->getApplication()->setDeviceOrientation(deviceOrientation_);


    
    Orientation orientation = application_->orientation();

    bool b1 = orientation == ePortrait || orientation == ePortraitUpsideDown;
    bool b2 = deviceOrientation_ == ePortrait || deviceOrientation_ == ePortraitUpsideDown;

    if (b1 != b2)
        hardwareOrientation_ = deviceOrientation_;
    else
        hardwareOrientation_ = orientation;

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
    if (ShaderEngine::Engine) ShaderEngine::Engine->resizeFramebuffer(width, height);
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
    return true;
}

static ApplicationManager *s_manager = NULL;

bool setKeyboardVisibility(bool visible){
    if (s_manager)
        return s_manager->setKeyboardVisibility(visible);
    return false;
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

void gdr_drawFrame()
{
	s_manager->drawFrame();
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
#if TARGET_OS_TV == 0
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
#endif
    
void gdr_keyDown(int keyCode, int repeat)
{
    s_manager->keyDown(keyCode,repeat);
}

void gdr_keyUp(int keyCode, int repeat)
{
    s_manager->keyUp(keyCode,repeat);
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

    
}

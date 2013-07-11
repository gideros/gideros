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

#include <soundimplementation.h>
#include <soundsystemopenal.h>
#include <sounddecoderwav.h>
#include <sounddecoderavaudioplayer.h>

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

#include "uitouchmanager.h"
#include "uitouchqueue.h"

#include <gui.h>

void g_setFps(int);
int g_getFps();
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
	}

	int scaleMode;
	int logicalWidth;
	int logicalHeight;
	std::vector<std::pair<std::string, float> > imageScales;
	int orientation;
	int fps;
	int retinaDisplay;
	int autorotation;
};

class ApplicationManager;

class NetworkManager
{
public:
	NetworkManager(ApplicationManager *application);
	~NetworkManager();
	void tick();
	
	void setResourceDirectory(const char* resourceDirectory)
	{
		resourceDirectory_ = resourceDirectory;
	}
	
	void setMd5FileName(const char *md5FileName)
	{
		md5filename_ = md5FileName;
		loadMD5();
	}
	
	static void printToServer_s(const char *str, void *data)
	{
		static_cast<NetworkManager*>(data)->printToServer(str);
	}
	
	void printToServer(const char *str)
	{
		unsigned int size = 1 + strlen(str) + 1;
		char* buffer = (char*)malloc(size);
		
		int pos = 0;
		buffer[pos] = 4;
		pos += 1;
		strcpy(buffer + pos, str);
		pos += strlen(str) + 1;
		
		server_->sendData(buffer, size);
		
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
	
	void luaError(const char *msg);
	
//	void surfaceCreated();
//	void surfaceChanged(int width, int height);
	void drawFrame();
	
//	void setDirectories(const char *externalDir, const char *internalDir, const char *cacheDir);
//	void setFileSystem(const char *files);
	
	void play(const std::vector<std::string>& luafiles);
	void stop();
	void setProjectName(const char *projectName);
	void setProjectProperties(const ProjectProperties &properties);
	
	void processTouches();
	void touchesBegan(NSSet *touches, NSSet *allTouches);
	void touchesMoved(NSSet *touches, NSSet *allTouches);
	void touchesEnded(NSSet *touches, NSSet *allTouches);
	void touchesCancelled(NSSet *touches, NSSet *allTouches);
	
	void suspend();
	void resume();
	
	static void *renderLoop_s(void *args);
	void *renderLoop();
	
	void exitRenderLoop();
	
	void didReceiveMemoryWarning();
	
	BOOL shouldAutorotateToInterfaceOrientation(UIInterfaceOrientation interfaceOrientation);	
	void willRotateToInterfaceOrientation(UIInterfaceOrientation toInterfaceOrientation);
	void didRotateFromInterfaceOrientation(UIInterfaceOrientation fromInterfaceOrientation);

private:
	void loadProperties();
	void loadLuaFiles();
	void drawIPs();
	int convertKeyCode(int keyCode);
	
private:
	UIView *view_;
	bool player_;
	LuaApplication *application_;
	NetworkManager *networkManager_;
	
	SoundImplementation *soundImplementation_;

	bool running_;
	
	int width_, height_;
	
	SplashScreen *splashScreen_;

	ProjectProperties properties_;
	
	Orientation hardwareOrientation_;

	bool luaFilesLoaded_;

	UITouchManager *uiTouchManager_;
	TouchQueue *uiTouchQueue_;

	pthread_mutex_t renderMutex_, autorotationMutex_;
	pthread_cond_t renderCond_;
	pthread_t renderThread_;
	
	bool renderLoopActive_;
	bool renderTick_;
	
	bool autorotating_;
	
	int nframe_;
};


NetworkManager::NetworkManager(ApplicationManager* application)
{
	application_ = application;
	server_ = new Server(15000);
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
				case 2:
					play(data);
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
				case 11:
					setProperties(data);
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

	// http
	ghttp_init();

	// sound
	soundImplementation_ = new SoundImplementation;
	soundImplementation_->addSoundSystem<SoundSystemOpenAL>();
	
	setSoundInterface(soundImplementation_);
	
	initializeSound();
	
	SoundDecoder* wav = soundImplementation_->addSoundDecoder<SoundDecoderWav>(1);
	soundImplementation_->addExtension("wav", wav);
	
	SoundDecoder* mp3 = soundImplementation_->addSoundDecoder<SoundDecoderAVAudioPlayer>(0x80000000);
	soundImplementation_->addExtension("mp3", mp3);
	
	// ui
	gui_init();

	// touch
	uiTouchManager_ = new UITouchManager;	
	uiTouchQueue_ = new TouchQueue;

	// network
	if (player_)
		networkManager_ = new NetworkManager(this);
	else
		networkManager_ = NULL;
	
	// application
	application_ = new LuaApplication;
	if (player_)
		application_->setPrintFunc(NetworkManager::printToServer_s, networkManager_);
	application_->enableExceptions();
	application_->initialize();
	application_->setResolution(width_, height_);
	
	Binder::disableTypeChecking();
	
	hardwareOrientation_ = ePortrait;

	running_ = false;

	renderLoopActive_ = true;
	renderTick_ = false;
	pthread_mutex_init(&renderMutex_, NULL);
	pthread_mutex_init(&autorotationMutex_, NULL);
	pthread_cond_init(&renderCond_, NULL);
	pthread_create(&renderThread_, NULL, renderLoop_s, this);
	
	autorotating_ = false;
	
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
		
		setDocumentsDirectory([documentsDirectory UTF8String]);
		setTemporaryDirectory([temporaryDirectory UTF8String]);
		setResourceDirectory(pathForFileEx([resourceDirectory UTF8String], "assets"));		

		loadProperties();
		
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
	}

	[view_ setFramebuffer];
	application_->clearBuffers();
	application_->renderScene(1);
	drawIPs();
	[view_ presentFramebuffer]; 
}

ApplicationManager::~ApplicationManager()
{	
	// application
	delete application_;

	// network
	if (networkManager_)
		delete networkManager_;
	
	// touch
	delete uiTouchQueue_;
	delete uiTouchManager_;
	
	// ui
	gui_cleanup();

	// sound
	deinitializeSound();
	setSoundInterface(NULL);
	delete soundImplementation_;
	soundImplementation_ = NULL;	
	
	// http
	ghttp_cleanup();
	
	// gpath & gvfs
	gvfs_cleanup();

	gpath_cleanup();
}

void *ApplicationManager::renderLoop_s(void *args)
{
	return static_cast<ApplicationManager*>(args)->renderLoop();
}

void *ApplicationManager::renderLoop()
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	
	printf("starting render thread\n");

    while (renderLoopActive_)
    {
        pthread_mutex_lock(&renderMutex_);
        while (!renderTick_)
            pthread_cond_wait(&renderCond_, &renderMutex_);

        pthread_mutex_lock(&autorotationMutex_);

		[view_ setFramebuffer];
		application_->clearBuffers();
		application_->renderScene(1);
		drawIPs();
		[view_ presentFramebuffer];        
		
        pthread_mutex_unlock(&autorotationMutex_);

		renderTick_ = false;
        pthread_mutex_unlock(&renderMutex_);
    }	
	
	
	printf("ending render thread\n");
	
	[pool release];
	
	return NULL;
}

void ApplicationManager::drawFrame()
{
	if (autorotating_)
		return;
	
	nframe_++;
	
	int frameSkip = g_getFps() == 30 ? 2 : 1;
	
	if ((nframe_ % frameSkip) != 0)
		return;	
	
	pthread_mutex_lock(&renderMutex_);
	
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

		pthread_mutex_unlock(&renderMutex_);
		
		return;
	}

	if (application_->isErrorSet())
		luaError(application_->getError());

	processTouches();
		
	{
		try
		{
			application_->enterFrame();
		}
		catch (LuaException e)
		{
			luaError(e.what());
		}
	}
    
	renderTick_ = true;
    pthread_cond_signal(&renderCond_);
	
	pthread_mutex_unlock(&renderMutex_);
}
	

void ApplicationManager::luaError(const char *error)
{
	g_log("%s", error);
	
	if (player_ == true)
	{
		running_ = false;
		
		networkManager_->printToServer(error);
		networkManager_->printToServer("\n");
		application_->deinitialize();
		application_->initialize();	
	}
	else
	{
		g_exit();
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
	
	bool phone = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone;
	bool notRetina = (properties_.retinaDisplay == 0) || (properties_.retinaDisplay == 1 && !phone) || (properties_.retinaDisplay == 2 && phone);
	
	float contentScaleFactor = 1;
	[view_ enableRetinaDisplay:(notRetina ? NO : YES)];
	if ([view_ respondsToSelector:@selector(contentScaleFactor)] == YES)
		contentScaleFactor = view_.contentScaleFactor;
	application_->setResolution(width_ * contentScaleFactor, height_ * contentScaleFactor);	
	application_->setHardwareOrientation(hardwareOrientation_);
	application_->setOrientation((Orientation)properties_.orientation);
	application_->setLogicalDimensions(properties_.logicalWidth, properties_.logicalHeight);
	application_->setLogicalScaleMode((LogicalScaleMode)properties_.scaleMode);
	application_->setImageScales(properties_.imageScales);
	
	g_setFps(properties_.fps);
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
	
	for (size_t i = 0; i < luafiles.size(); ++i)
	{
		try
		{
			application_->loadFile(luafiles[i].c_str());
		}
		catch (LuaException e)
		{
			luaError(e.what());
		}
	}
	
	Event event(Event::APPLICATION_START);
	application_->broadcastEvent(&event);
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
	application_->setOrientation((Orientation)properties_.orientation);
	application_->setLogicalDimensions(properties_.logicalWidth, properties_.logicalHeight);
	application_->setLogicalScaleMode((LogicalScaleMode)properties_.scaleMode);
	application_->setImageScales(properties_.imageScales);
	
	g_setFps(properties_.fps);
	
	try
	{
		for (std::size_t i = 0; i < luafiles.size(); ++i)
			application_->loadFile(luafiles[i].c_str());
		
		Event event(Event::APPLICATION_START);
		application_->broadcastEvent(&event);
	}
	catch (LuaException e)
	{
		luaError(e.what());
	}
}

void ApplicationManager::stop()
{
	if (running_ == true)
	{
		try
		{
			Event event(Event::APPLICATION_EXIT);
			application_->broadcastEvent(&event);
		}
		catch (LuaException e)
		{
			luaError(e.what());
		}
	}
	
	running_ = false;
	
	application_->deinitialize();
	application_->initialize();
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
	g_log("setProjectName: %s", projectName);
	
	NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
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
	
	g_log("documents: %s", documents.c_str());
	g_log("temporary: %s", temporary.c_str());
	g_log("resource: %s", resource.c_str());
	
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
	pthread_mutex_lock(&renderMutex_);	
	try
	{
		Event event(Event::APPLICATION_SUSPEND);
		application_->broadcastEvent(&event);
	}
	catch (LuaException e)
	{
		luaError(e.what());
	}
	pthread_mutex_unlock(&renderMutex_);
}

void ApplicationManager::resume()
{
	pthread_mutex_lock(&renderMutex_);
	try
	{
		Event event(Event::APPLICATION_RESUME);
		application_->broadcastEvent(&event);
	}
	catch (LuaException e)
	{
		luaError(e.what());
	}
	pthread_mutex_unlock(&renderMutex_);
}

void ApplicationManager::exitRenderLoop()
{
	pthread_mutex_lock(&renderMutex_);
	renderLoopActive_ = false;
	renderTick_ = true;
    pthread_cond_signal(&renderCond_);	
	pthread_mutex_unlock(&renderMutex_);

	pthread_join(renderThread_, NULL);
	
	try
	{
		Event event(Event::APPLICATION_EXIT);
		application_->broadcastEvent(&event);
	}
	catch (LuaException e)
	{
		luaError(e.what());
	}		
}

void ApplicationManager::touchesBegan(NSSet *touches, NSSet *allTouches)
{
	NSArray* touchesArray = [touches allObjects];
	static std::vector<UITouch*> uiTouches;
	uiTouches.clear();
	for (int i = 0; i < [touchesArray count]; ++i)
	{
		UITouch* touch = [touchesArray objectAtIndex:i];
		uiTouches.push_back(touch);
	}
	
	NSArray* allTouchesArray = [allTouches allObjects];
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
	
	uiTouchManager_->update((UIView*)view_, uiTouches, uiAllTouches, &touchesSet, &allTouchesSet);
	uiTouchQueue_->push(0, touchesSet);
	uiTouchQueue_->push(0, allTouchesSet);
}

void ApplicationManager::touchesMoved(NSSet *touches, NSSet *allTouches)
{
	NSArray* touchesArray = [touches allObjects];
	static std::vector<UITouch*> uiTouches;
	uiTouches.clear();
	for (int i = 0; i < [touchesArray count]; ++i)
	{
		UITouch* touch = [touchesArray objectAtIndex:i];
		uiTouches.push_back(touch);
	}
	
	NSArray* allTouchesArray = [allTouches allObjects];
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
	
	uiTouchManager_->update((UIView*)view_, uiTouches, uiAllTouches, &touchesSet, &allTouchesSet);
	uiTouchQueue_->push(1, touchesSet);
	uiTouchQueue_->push(1, allTouchesSet);
}

void ApplicationManager::touchesEnded(NSSet *touches, NSSet *allTouches)
{
	NSArray* touchesArray = [touches allObjects];
	static std::vector<UITouch*> uiTouches;
	uiTouches.clear();
	for (int i = 0; i < [touchesArray count]; ++i)
	{
		UITouch* touch = [touchesArray objectAtIndex:i];
		uiTouches.push_back(touch);
	}
	
	NSArray* allTouchesArray = [allTouches allObjects];
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
	
	uiTouchManager_->update((UIView*)view_, uiTouches, uiAllTouches, &touchesSet, &allTouchesSet);
	uiTouchQueue_->push(2, touchesSet);
	uiTouchQueue_->push(2, allTouchesSet);
}

void ApplicationManager::touchesCancelled(NSSet *touches, NSSet *allTouches)
{
	NSArray* touchesArray = [touches allObjects];
	static std::vector<UITouch*> uiTouches;
	uiTouches.clear();
	for (int i = 0; i < [touchesArray count]; ++i)
	{
		UITouch* touch = [touchesArray objectAtIndex:i];
		uiTouches.push_back(touch);
	}
	
	NSArray* allTouchesArray = [allTouches allObjects];
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
	
	uiTouchManager_->update((UIView*)view_, uiTouches, uiAllTouches, &touchesSet, &allTouchesSet);
	uiTouchQueue_->push(3, touchesSet);
	uiTouchQueue_->push(3, allTouchesSet);
}

void ApplicationManager::processTouches()
{
	try
	{
		while (!uiTouchQueue_->empty())
		{
			TouchQueueElement* touches = uiTouchQueue_->front();
			uiTouchQueue_->pop();
			
			TouchQueueElement* allTouches = uiTouchQueue_->front();
			uiTouchQueue_->pop();
			
			switch (touches->type)
			{
				case 0:
					if (application_)
						application_->touchesBegan(touches->touches, allTouches->touches);
					break;
				case 1:
					if (application_)
						application_->touchesMoved(touches->touches, allTouches->touches);
					break;
				case 2:
					if (application_)
						application_->touchesEnded(touches->touches, allTouches->touches);
					break;
				case 3:
					if (application_)
						application_->touchesCancelled(touches->touches, allTouches->touches);
					break;
			}
		}
	}
	catch (LuaException e)
	{
		luaError(e.what());
	}
}

void ApplicationManager::didReceiveMemoryWarning()
{
	pthread_mutex_lock(&renderMutex_);
	application_->collectGarbage();
	application_->collectGarbage();	
	pthread_mutex_unlock(&renderMutex_);
}

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
	
	if (result == YES)
	{
		if (dontAutorotate)
			hardwareOrientation_ = ePortrait;
		else
			hardwareOrientation_ = application_->orientation();
		application_->setHardwareOrientation(hardwareOrientation_);
	}
	
	return result;
}

void ApplicationManager::willRotateToInterfaceOrientation(UIInterfaceOrientation toInterfaceOrientation)
{
	pthread_mutex_lock(&autorotationMutex_);
	autorotating_ = true;
	
	bool phone = UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone;
	bool dontAutorotate = (properties_.autorotation == 0) || (properties_.autorotation == 1 && !phone) || (properties_.autorotation == 2 && phone);
	
	if (dontAutorotate)
		hardwareOrientation_ = ePortrait;
	else
		hardwareOrientation_ = application_->orientation();
	application_->setHardwareOrientation(hardwareOrientation_);
}

void ApplicationManager::didRotateFromInterfaceOrientation(UIInterfaceOrientation fromInterfaceOrientation)
{
	autorotating_ = false;
	pthread_mutex_unlock(&autorotationMutex_);
}

static ApplicationManager *s_manager = NULL;

void gdr_initialize(UIView* view, int width, int height, bool player)
{
	s_manager = new ApplicationManager(view, width, height, player);
}

void gdr_drawFrame()
{
	s_manager->drawFrame();
}

void gdr_resize(int width, int height)
{
}

void gdr_exitGameLoop()
{
	s_manager->exitRenderLoop();
}


void gdr_deinitialize()
{
	delete s_manager;
	s_manager = NULL;
}

void gdr_suspend()
{
	s_manager->suspend();
}

void gdr_resume()
{
	s_manager->resume();
}

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

void gdr_didReceiveMemoryWarning()
{
	s_manager->didReceiveMemoryWarning();
}

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

void gdr_setAccelerometer(double x, double y, double z)
{
	::setAccelerometer(x, y, z);
}


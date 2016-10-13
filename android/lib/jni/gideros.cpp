#include <jni.h>
#include <android/log.h>
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
#include <gvfs-android.h>

#include <pystring.h>

#include <javanativebridge.h>

#include <GLES/gl.h>

#include <glog.h>

#include <keycode.h>

#include <binder.h>

#include <gui.h>
#include <ginput.h>
#include <ginput-android.h>
#include <gevent.h>
#include <ggeolocation.h>

#include <glog.h>

#include <gtexture.h>

#include <gaudio.h>

#include <gapplication.h>
#include <gapplication-android.h>


extern "C" {
int g_getFps();
void g_setFps(int fps);
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
		static_cast<NetworkManager*>(data)->printToServer(str, len);
	}

	void printToServer(const char *str, int len)
	{
		unsigned int size = 1 + ((len < 0) ? strlen(str) : len) + 1;
		char* buffer = (char*)malloc(size);

	    buffer[0] = 4;
	    memcpy(buffer + 1, str,size-2);
	    buffer[size-1]=0;

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
	ApplicationManager(JNIEnv *env, bool player);
	~ApplicationManager();

	void luaError(const char *msg);
	
	void surfaceCreated();
	void surfaceChanged(int width, int height, int rotation);
	void updateHardwareOrientation();
	void drawFrame();

	void setDirectories(const char *externalDir, const char *internalDir, const char *cacheDir);
	void setFileSystem(const char *files);
	
	void openProject(const char* project);
	void setOpenProject(const char* project);
	void play(const std::vector<std::string>& luafiles);
	void stop();
	void setProjectName(const char *projectName);
	void setProjectProperties(const ProjectProperties &properties);
	bool isRunning();
	
	void touchesBegin(int size, int *id, int *x, int *y, float *pressure, int actionIndex);
	void touchesMove(int size, int *id, int *x, int *y, float *pressure);
	void touchesEnd(int size, int *id, int *x, int *y, float *pressure, int actionIndex);
	void touchesCancel(int size, int *id, int *x, int *y, float *pressure);
	
	bool keyDown(int keyCode, int repeatCount);
	bool keyUp(int keyCode, int repeatCount);
	void keyChar(const char *keyChar);
	
	void pause();
	void resume();

	void lowMemory();

	void background();
	void foreground();	
	
private:
	void loadProperties();
	void loadLuaFiles();
	void drawIPs();
	int convertKeyCode(int keyCode);
	
private:
	bool player_;
	LuaApplication *application_;
	NetworkManager *networkManager_;
	
	bool init_;
	
	bool running_;
	
	int width_, height_;

	SplashScreen *splashScreen_;
	
	std::string externalDir_, internalDir_, cacheDir_;
	
	ProjectProperties properties_;
	
	Orientation hardwareOrientation_;
	
	Orientation deviceOrientation_;
	
	int nframe_;
	
	bool applicationStarted_;
	
	bool skipFirstEnterFrame_;
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
					fclose(fos);
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

static void printToLog_s(const char *str, int len, void *data)
{
	if (len<0)
		glog_i("%s\n",str);
	else
	{
		char* buffer = (char*)malloc(len+1);
		memcpy(buffer, str,len);
		buffer[len]=0;
		glog_i("%s\n",buffer);
		free(buffer);
	}
}

ApplicationManager::ApplicationManager(JNIEnv *env, bool player)
{
	JavaVM* vm;
	env->GetJavaVM(&vm);
	jnb_setJavaVM(vm);

	player_ = player;
	
	// gpath & gvfs
	gpath_init();
    gpath_addDrivePrefix(0, "|R|");
    gpath_addDrivePrefix(0, "|r|");
    gpath_addDrivePrefix(1, "|D|");
    gpath_addDrivePrefix(1, "|d|");
    gpath_addDrivePrefix(2, "|T|");
    gpath_addDrivePrefix(2, "|t|");

    gpath_setDriveFlags(0, GPATH_RO);
    gpath_setDriveFlags(1, GPATH_RW | GPATH_REAL);
    gpath_setDriveFlags(2, GPATH_RW | GPATH_REAL);
	
	gpath_setAbsolutePathFlags(GPATH_RW | GPATH_REAL);

    gpath_setDefaultDrive(0);
	
	gvfs_init();
	gvfs_setPlayerModeEnabled(player ? 1 : 0);

	// event
	gevent_Init();
	
	// application
	gapplication_init();
	
	// input
	ginput_init();
	
	// geolocation
	ggeolocation_init();
	
	// http
	ghttp_Init();
	
	// ui
	gui_init();

	// texture
    gtexture_init();
    gtexture_setCachingEnabled(1);
	
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
	else
		application_->setPrintFunc(printToLog_s, NULL);
	application_->enableExceptions();

	Binder::disableTypeChecking();
		
	init_ = false;
	running_ = false;
	
	width_ = 0;
	height_ = 0;
	
	splashScreen_ = NULL;
	
	nframe_ = 0;
	
	applicationStarted_ = false;
	
	skipFirstEnterFrame_ = false;
}

ApplicationManager::~ApplicationManager()
{
	// network
	if (networkManager_)
		delete networkManager_;

	// application
	application_->deinitialize();
	delete application_;
	
	// audio
    gaudio_Cleanup();

	// texture
    gtexture_cleanup();

	// ui
	gui_cleanup();

	// http
	ghttp_Cleanup();
	
	// geolocation
	ggeolocation_cleanup();

	// input
	ginput_cleanup();

	// application
	gapplication_cleanup();
	
	// event
	gevent_Cleanup();
	
	// gpath & gvfs
	gvfs_cleanup();
	gpath_cleanup();
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
		JNIEnv *env = g_getJNIEnv();
		jstring jerrormsg = env->NewStringUTF(error);
		jclass localRefCls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		jmethodID throwError = env->GetStaticMethodID(localRefCls, "throwLuaException", "(Ljava/lang/String;)V");
		env->CallStaticVoidMethod(localRefCls, throwError, jerrormsg);
		env->DeleteLocalRef(jerrormsg);
		env->DeleteLocalRef(localRefCls);
		//g_exit();
	}
}

void ApplicationManager::surfaceCreated()
{
	if (!init_)
	{
		init_ = true;
		application_->initialize();
	}
	else
	{
		if (ShaderEngine::Engine)
			ShaderEngine::Engine->reset(true);
		gtexture_reloadTextures();
		gtexture_RestoreRenderTargets();
		gtexture_RestoreTempTextures();
	}
}

void ApplicationManager::surfaceChanged(int width, int height, int rotation)
{
	if (player_ == true)
		refreshLocalIPs();

	if (width > height)
	{
		width_ = height;
		height_ = width;
	}
	else
	{
		width_ = width;
		height_ = height;
	}
	
	application_->setResolution(width_, height_);
	
	switch (rotation)
	{
	case 0:
		deviceOrientation_ = ePortrait;
		break;
	case 90:
		deviceOrientation_ = eLandscapeLeft;
		break;
	case 180:
		deviceOrientation_ = ePortraitUpsideDown;
		break;
	case 270:
		deviceOrientation_ = eLandscapeRight;
		break;
	default:
		deviceOrientation_ = ePortrait;
		break;
	}
	
	application_->getApplication()->setDeviceOrientation(deviceOrientation_);
	
	updateHardwareOrientation();
}

void ApplicationManager::updateHardwareOrientation()
{
    Orientation orientation = application_->orientation();

    bool b1 = orientation == ePortrait || orientation == ePortraitUpsideDown;
    bool b2 = deviceOrientation_ == ePortrait || deviceOrientation_ == ePortraitUpsideDown;

    if (b1 != b2)
        hardwareOrientation_ = deviceOrientation_;
    else
        hardwareOrientation_ = orientation;

    application_->setHardwareOrientation(hardwareOrientation_);	
}

void ApplicationManager::drawFrame()
{
	if (networkManager_)
		networkManager_->tick();

	// if we're in app mode, skip the first 10 frames as black screen
	// because a pattern like surfaceChanged(a,b), drawFrame, drawFrame, drawFrame, ..., surfaceChanged(b,a) is possible
	// and we want to execute lua files after getting the final device resolution
	if (player_ == false)
	{
		if (nframe_++ < 10)
		{
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			return;
		}
	}

	if (player_ == false)
	{
		if (applicationStarted_ == false)
		{
			loadProperties();

			// Gideros has became open source and free, because this, there's no more splash art			

			loadLuaFiles();
			skipFirstEnterFrame_ = true;

			/*
			bool licensed = (licenseKey_[15] == 'f' && licenseKey_[16] == 'f');

			if (licensed)
			{
				loadLuaFiles();
				skipFirstEnterFrame_ = true;
			}
			else
			{
				application_->getApplication()->setBackgroundColor(0, 0, 0);
				splashScreen_ = new SplashScreen(application_->getApplication());
				application_->getApplication()->stage()->addChild(splashScreen_);
				splashScreen_->unref();
			}
			*/

			applicationStarted_ = true;
		}
		
		if (splashScreen_ && splashScreen_->isFinished())
		{
			application_->getApplication()->stage()->removeChild(splashScreen_);
			splashScreen_ = NULL;
			application_->getApplication()->setBackgroundColor(1, 1, 1);

			loadLuaFiles();
			skipFirstEnterFrame_ = true;
		}
	}	

	if (skipFirstEnterFrame_ == true)
	{	
		skipFirstEnterFrame_ = false;
	}
	else
	{
		GStatus status;
		application_->enterFrame(&status);
		if (status.error())
			luaError(status.errorString());
	}

	application_->clearBuffers();
	application_->renderScene(1);
	drawIPs();
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

	
	application_->setResolution(width_, height_);
	application_->setOrientation((Orientation)properties_.orientation);
	updateHardwareOrientation();
	application_->getApplication()->setDeviceOrientation(deviceOrientation_);
	application_->setLogicalDimensions(properties_.logicalWidth, properties_.logicalHeight);
	application_->setLogicalScaleMode((LogicalScaleMode)properties_.scaleMode);
	application_->setImageScales(properties_.imageScales);

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

void ApplicationManager::setDirectories(const char *externalDir, const char *internalDir, const char *cacheDir)
{
	externalDir_ = externalDir;
	internalDir_ = internalDir;
	cacheDir_ = cacheDir;
}

void ApplicationManager::setFileSystem(const char *fileSystem)
{
	std::vector<std::string> result;
	pystring::split(fileSystem, result, "|");

	gvfs_setZipFiles(result[0].c_str(), result[1].c_str(), result[2].c_str());

	for (size_t i = 3; i < result.size(); i += 4)
	{
		gvfs_addFile(result[i].c_str(), atoi(result[i + 1].c_str()), atoi(result[i + 2].c_str()), atoi(result[i + 3].c_str()));
		glog_v("%s %d %d %d", result[i].c_str(), atoi(result[i + 1].c_str()), atoi(result[i + 2].c_str()), atoi(result[i + 3].c_str()));
	}

	setDocumentsDirectory(internalDir_.c_str());
	setTemporaryDirectory(cacheDir_.c_str());
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

void ApplicationManager::play(const std::vector<std::string>& luafiles)
{
	running_ = true;
	
	application_->deinitialize();
	application_->initialize();
	application_->setResolution(width_, height_);
	application_->setOrientation((Orientation)properties_.orientation);
	updateHardwareOrientation();
	application_->getApplication()->setDeviceOrientation(deviceOrientation_);
	application_->setLogicalDimensions(properties_.logicalWidth, properties_.logicalHeight);
	application_->setLogicalScaleMode((LogicalScaleMode)properties_.scaleMode);
	application_->setImageScales(properties_.imageScales);

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

	std::string dir = externalDir_;

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

void ApplicationManager::touchesBegin(int size, int *id, int *x, int *y, float *pressure, int actionIndex)
{
	ginputp_touchBegin(size, id, x, y, pressure, actionIndex);
}

void ApplicationManager::touchesMove(int size, int *id, int *x, int *y, float *pressure)
{
	ginputp_touchesMove(size, id, x, y, pressure);
}

void ApplicationManager::touchesEnd(int size, int *id, int *x, int *y, float *pressure, int actionIndex)
{
	ginputp_touchEnd(size, id, x, y, pressure, actionIndex);
}

void ApplicationManager::touchesCancel(int size, int *id, int *x, int *y, float *pressure)
{
	ginputp_touchesCancel(size, id, x, y, pressure);
}

bool ApplicationManager::keyDown(int keyCode, int repeatCount)
{
	int result = ginputp_keyDown(keyCode, repeatCount);
	if (result == 0)
		return false;

	if (player_ == true && running_ == false)
		return false;
		
	return true;
}

bool ApplicationManager::keyUp(int keyCode, int repeatCount)
{
	int result = ginputp_keyUp(keyCode, repeatCount);
	if (result == 0)
		return false;

	if (player_ == true && running_ == false)
		return false;

	return true;
}

void ApplicationManager::keyChar(const char *str)
{
	ginputp_keyChar(str);
}

extern void gaudio_android_suspend(bool suspend);

void ApplicationManager::pause()
{
	gtexture_SaveRenderTargets();

    gapplication_enqueueEvent(GAPPLICATION_PAUSE_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
	gaudio_android_suspend(true);
 }

void ApplicationManager::resume()
{
	gaudio_android_suspend(false);
    gapplication_enqueueEvent(GAPPLICATION_RESUME_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
 }

void ApplicationManager::lowMemory()
{
    gapplication_enqueueEvent(GAPPLICATION_MEMORY_LOW_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
}

void ApplicationManager::background()
{
    gapplication_enqueueEvent(GAPPLICATION_BACKGROUND_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
 }

void ApplicationManager::foreground()
{
    gapplication_enqueueEvent(GAPPLICATION_FOREGROUND_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
 }

static ApplicationManager *s_applicationManager = NULL;

extern "C" {

int GiderosOpenALConfig_sampleRate=44100;

void Java_com_giderosmobile_android_player_GiderosApplication_nativeOpenALSetup(JNIEnv *env, jclass cls, jint sampleRate)
{
	GiderosOpenALConfig_sampleRate=sampleRate;
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeCreate(JNIEnv *env, jclass cls, jboolean player)
{
	if (s_applicationManager != NULL)
		delete s_applicationManager;
	s_applicationManager = new ApplicationManager(env, player != 0);
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeDestroy(JNIEnv *env, jclass cls)
{
	delete s_applicationManager;
	s_applicationManager = NULL;
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeSurfaceCreated(JNIEnv *env, jclass cls)
{
	s_applicationManager->surfaceCreated();
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeSurfaceChanged(JNIEnv *env, jclass cls, jint width, jint height, jint rotation)
{
	s_applicationManager->surfaceChanged(width, height, rotation);
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeDrawFrame(JNIEnv *env, jclass cls)
{
	s_applicationManager->drawFrame();
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeSetDirectories(JNIEnv *env, jclass cls, jstring jExternalDir, jstring jInternalDir, jstring jCacheDir)
{
    const char *szExternalDir = env->GetStringUTFChars(jExternalDir, NULL);
    std::string externalDir = szExternalDir;
    env->ReleaseStringUTFChars(jExternalDir, szExternalDir);

    const char *szInternalDir = env->GetStringUTFChars(jInternalDir, NULL);
    std::string internalDir = szInternalDir;
    env->ReleaseStringUTFChars(jInternalDir, szInternalDir);

    const char *szCacheDir = env->GetStringUTFChars(jCacheDir, NULL);
    std::string cacheDir = szCacheDir;
    env->ReleaseStringUTFChars(jCacheDir, szCacheDir);

	s_applicationManager->setDirectories(externalDir.c_str(), internalDir.c_str(), cacheDir.c_str());
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeSetFileSystem(JNIEnv *env, jclass cls, jstring jFiles)
{
	const char *szFiles = env->GetStringUTFChars(jFiles, NULL);
	std::string files = szFiles;
	env->ReleaseStringUTFChars(jFiles, szFiles);
	
	s_applicationManager->setFileSystem(files.c_str());
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeOpenProject(JNIEnv *env, jclass cls, jstring jProject)
{
	const char *sproject = env->GetStringUTFChars(jProject, NULL);
	std::string project = sproject;
	env->ReleaseStringUTFChars(jProject, sproject);
	
	s_applicationManager->setOpenProject(project.c_str());
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeTouchesBegin(JNIEnv* env, jobject thiz, jint size, jintArray jid, jintArray jx, jintArray jy, jfloatArray jpressure, jint actionIndex)
{
	jint* id = (jint*)env->GetPrimitiveArrayCritical(jid, 0);
	jint* x = (jint*)env->GetPrimitiveArrayCritical(jx, 0);
	jint* y = (jint*)env->GetPrimitiveArrayCritical(jy, 0);
	jfloat* pressure = (jfloat*)env->GetPrimitiveArrayCritical(jpressure, 0);

	s_applicationManager->touchesBegin(size, id, x, y, pressure, actionIndex);

	env->ReleasePrimitiveArrayCritical(jid, id, 0);
	env->ReleasePrimitiveArrayCritical(jx, x, 0);
	env->ReleasePrimitiveArrayCritical(jy, y, 0);
	env->ReleasePrimitiveArrayCritical(jpressure, pressure, 0);
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeTouchesMove(JNIEnv* env, jobject thiz, jint size, jintArray jid, jintArray jx, jintArray jy, jfloatArray jpressure)
{
	jint* id = (jint*)env->GetPrimitiveArrayCritical(jid, 0);
	jint* x = (jint*)env->GetPrimitiveArrayCritical(jx, 0);
	jint* y = (jint*)env->GetPrimitiveArrayCritical(jy, 0);
	jfloat* pressure = (jfloat*)env->GetPrimitiveArrayCritical(jpressure, 0);

	s_applicationManager->touchesMove(size, id, x, y, pressure);

	env->ReleasePrimitiveArrayCritical(jid, id, 0);
	env->ReleasePrimitiveArrayCritical(jx, x, 0);
	env->ReleasePrimitiveArrayCritical(jy, y, 0);
	env->ReleasePrimitiveArrayCritical(jpressure, pressure, 0);
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeTouchesEnd(JNIEnv* env, jobject thiz, jint size, jintArray jid, jintArray jx, jintArray jy, jfloatArray jpressure, jint actionIndex)
{
	jint* id = (jint*)env->GetPrimitiveArrayCritical(jid, 0);
	jint* x = (jint*)env->GetPrimitiveArrayCritical(jx, 0);
	jint* y = (jint*)env->GetPrimitiveArrayCritical(jy, 0);
	jfloat* pressure = (jfloat*)env->GetPrimitiveArrayCritical(jpressure, 0);

	s_applicationManager->touchesEnd(size, id, x, y, pressure, actionIndex);

	env->ReleasePrimitiveArrayCritical(jid, id, 0);
	env->ReleasePrimitiveArrayCritical(jx, x, 0);
	env->ReleasePrimitiveArrayCritical(jy, y, 0);
	env->ReleasePrimitiveArrayCritical(jpressure, pressure, 0);
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeTouchesCancel(JNIEnv* env, jobject thiz, jint size, jintArray jid, jintArray jx, jintArray jy, jfloatArray jpressure)
{
	jint* id = (jint*)env->GetPrimitiveArrayCritical(jid, 0);
	jint* x = (jint*)env->GetPrimitiveArrayCritical(jx, 0);
	jint* y = (jint*)env->GetPrimitiveArrayCritical(jy, 0);
	jfloat* pressure = (jfloat*)env->GetPrimitiveArrayCritical(jpressure, 0);

	s_applicationManager->touchesCancel(size, id, x, y, pressure);

	env->ReleasePrimitiveArrayCritical(jid, id, 0);
	env->ReleasePrimitiveArrayCritical(jx, x, 0);
	env->ReleasePrimitiveArrayCritical(jy, y, 0);
	env->ReleasePrimitiveArrayCritical(jpressure, pressure, 0);
}

jboolean Java_com_giderosmobile_android_player_GiderosApplication_nativeKeyDown(JNIEnv* env, jclass cls, jint keyCode, jint repeatCount)
{
	return s_applicationManager->keyDown(keyCode, repeatCount);
}

jboolean Java_com_giderosmobile_android_player_GiderosApplication_nativeKeyUp(JNIEnv* env, jclass cls, jint keyCode, jint repeatCount)
{
	return s_applicationManager->keyUp(keyCode, repeatCount);
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeKeyChar(JNIEnv* env, jclass cls, jstring keyChar)
{
	const char* sBytes = env->GetStringUTFChars(keyChar, NULL);
	s_applicationManager->keyChar(sBytes);
	env->ReleaseStringUTFChars(keyChar, sBytes);
}

jboolean Java_com_giderosmobile_android_player_GiderosApplication_isRunning(JNIEnv* env, jclass cls)
{
	return s_applicationManager->isRunning();
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativePause(JNIEnv* env, jclass cls)
{
	s_applicationManager->pause();
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeResume(JNIEnv* env, jclass cls)
{
	s_applicationManager->resume();
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeLowMemory(JNIEnv* env, jclass cls)
{
	if (s_applicationManager)
		s_applicationManager->lowMemory();
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeStop(JNIEnv* env, jclass cls)
{
	s_applicationManager->background();
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeStart(JNIEnv* env, jclass cls)
{
	s_applicationManager->foreground();
}


}


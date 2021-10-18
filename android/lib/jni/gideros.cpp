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
#include <gvfs-native.h>

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
#include "debugging.h"

extern "C" {
int g_getFps();
void g_setFps(int fps);
}
void drawInfo();
void drawInfoResolution(int width, int height, int scale, int lWidth,
		int lHeight, bool drawRunning, float canvasColor[3],
		float infoColor[3],int ho, int ao, float fps, float cpu);
void refreshLocalIPs();
void g_exit();

static volatile const char* licenseKey_ = "9852564f4728e0c11e34ca3eb5fe20b2";
//-----------------------------------------01234567890123456------------------


#ifdef OCULUS
#include "oculus/oculus.h"
#define _OCULUS(f,...) oculus::f( __VA_ARGS__);
#else
#define _OCULUS(...) ;
#endif
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

class CThreadLock
{
public:
    CThreadLock();
    virtual ~CThreadLock();

    void Lock();
    void Unlock();
private:
    pthread_mutex_t mutexlock;
    pthread_mutexattr_t attr;
};

CThreadLock::CThreadLock()
{
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mutexlock, &attr);
}

CThreadLock::~CThreadLock()
{
    // deinit lock here
    pthread_mutex_destroy(&mutexlock);
}
void CThreadLock::Lock()
{
    // lock
    pthread_mutex_lock(&mutexlock);
}
void CThreadLock::Unlock()
{
    // unlock
    pthread_mutex_unlock(&mutexlock);
}


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
	LuaApplication *getApplication() { return application_;};

	void setDirectories(const char *externalDir, const char *internalDir, const char *cacheDir);
	void setFileSystem(const char *files);
	
	void openProject(const char* project);
	void setOpenProject(const char* project);
	void play(const std::vector<std::string>& luafiles);
	void stop();
	void setProjectName(const char *projectName);
	void setProjectProperties(const ProjectProperties &properties);
	bool isRunning();

#ifdef OCULUS
	void oculusTick(double elapsed);
	void oculusRender(float *vmat,float *pmat,int width, int height,bool room,bool screen,bool floor);
	void oculusInputEvent(oculus::Input &input);
#endif

	void mouseWheel(int x,int y,int button,float amount);
	void touchesBegin(int size, int *id, int *x, int *y, float *pressure, int actionIndex);
	void touchesMove(int size, int *id, int *x, int *y, float *pressure);
	void touchesEnd(int size, int *id, int *x, int *y, float *pressure, int actionIndex);
	void touchesCancel(int size, int *id, int *x, int *y, float *pressure);
	
	bool keyDown(int keyCode, int repeatCount);
	bool keyUp(int keyCode, int repeatCount);
	void keyChar(const char *keyChar);
	void textInput(const char *text,int ss,int se);

	void pause();
	void resume();

	void lowMemory();
	void handleOpenUrl(const char *url);

	void background();
	void foreground();	
	void forceTick();
	
private:
	void loadProperties();
	void loadLuaFiles();
	void drawIPs();
	int convertKeyCode(int keyCode);
	CThreadLock tickLock;
	
private:
	bool player_;
	LuaApplication *application_;
	NetworkManager *networkManager_;
	
	bool init_;
	
	bool running_;
	bool paused_;
	
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
		glog(str);
	else
	{
		char* buffer = (char*)malloc(len+1);
		memcpy(buffer, str,len);
		buffer[len]=0;
		glog(buffer);
		free(buffer);
	}
}

extern void eventFlush();

ApplicationManager::ApplicationManager(JNIEnv *env, bool player)
{
	JavaVM* vm;
	env->GetJavaVM(&vm);
	jnb_setJavaVM(vm);

	player_ = player;
	paused_ = false;
	
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
	gevent_SetFlusher(eventFlush);
	
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
		_OCULUS(onLuaReinit);
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
		_OCULUS(onLuaReinit);
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
	
	bool keepBuffers=false;
#ifndef OCULUS
	keepBuffers=true;
#endif
	application_->setResolution(width_, height_,keepBuffers);
	
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

	tickLock.Lock();
	Event event(Event::APPLICATION_RESIZE);
	GStatus status;
	application_->broadcastEvent(&event, &status);
   	tickLock.Unlock();
	if (status.error())
		luaError(status.errorString());
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

#ifdef OCULUS
static const char VERTEX_SHADER[] =
    "attribute highp vec3 vVertex;\n"
    "attribute lowp vec4 vertexColor;\n"
    "attribute highp vec3 vertexNormal;\n"
	"uniform highp mat4 g_MVPMatrix;\n"
	"uniform highp mat4 g_MVMatrix;\n"
	"uniform highp mat4 g_NMatrix;\n"
	"varying highp vec3 fragmentPosition;\n"
	"varying lowp vec4 fragmentColor;\n"
	"varying highp vec3 fragmentNormal;\n"
    "void main()\n"
    "{\n"
	"	fragmentPosition = (g_MVMatrix*vec4(vVertex.xyz,0.0)).xyz;\n"
	"	fragmentNormal = normalize((g_NMatrix*vec4(vertexNormal.xyz,0.0)).xyz);\n"
    "	fragmentColor = vertexColor;\n"
	"   highp vec4 vertex = vec4(vVertex,1.0);\n"
	"   gl_Position = g_MVPMatrix*vertex;\n"
    "}\n";

static const char FRAGMENT_SHADER[] =
	"varying highp vec3 fragmentPosition;\n"
	"varying lowp vec4 fragmentColor;\n"
	"varying highp vec3 fragmentNormal;\n"
    "void main()\n"
    "{\n"
		"	lowp vec3 color0 = fragmentColor.xyz;\n"
		"	lowp vec3 color1 = vec3(0.5, 0.5, 0.5);\n"
		"	highp vec3 normal = fragmentNormal;\n"
		"	highp vec3 lightPos = vec3(0,50,50);\n"
		"	highp vec3 lightDir = normalize(lightPos.xyz - fragmentPosition.xyz);\n"
		"	highp vec3 viewDir = normalize(-fragmentPosition.xyz);\n"
		"	mediump float ambient=0.4;\n"
		"	mediump float diff = max(0.0, dot(normal, lightDir));\n"
		"	mediump float spec =0.0;\n"
		"	if (diff>0.0)\n"
		"	{\n"
		"		mediump float nh = max(0.0, dot(reflect(-lightDir,normal),viewDir));\n"
		"		spec = pow(nh, 96.0);\n"
		"	}\n"
		"	diff=max(diff,ambient);\n"

		"	gl_FragColor = vec4(color0 * diff + color1 * spec, 1);\n"
    "}\n";

const ShaderProgram::ConstantDesc stdUniforms[] = {
		{ "g_MVPMatrix",ShaderProgram::CMATRIX, 1,	ShaderProgram::SysConst_WorldViewProjectionMatrix, true, 0, NULL },
		{ "g_MVMatrix",ShaderProgram::CMATRIX, 1,	ShaderProgram::SysConst_WorldMatrix, true, 0, NULL },
		{ "g_NMatrix",ShaderProgram::CMATRIX, 1,	ShaderProgram::SysConst_WorldInverseTransposeMatrix3, true, 0, NULL },
		{ "",ShaderProgram::CFLOAT, 0, ShaderProgram::SysConst_None,false, 0, NULL } };
const ShaderProgram::DataDesc stdAttributes[] = {
		{ "vVertex",	ShaderProgram::DFLOAT, 3, 0, 0,0 },
		{ "vertexColor", ShaderProgram::DUBYTE,	4, 1, 0,0 },
		{ "vTexCoord", ShaderProgram::DFLOAT, 2, 2, 0,0 },
		{ "vertexNormal", ShaderProgram::DFLOAT, 3, 3, 0,0 },
		{ "",ShaderProgram::DFLOAT, 0, 0, 0,0 } };

static ShaderProgram *cachedLS=NULL;
ShaderProgram *getLightingShader(ShaderEngine *gfx) {
	if (cachedLS) return cachedLS;
	cachedLS=gfx->createShaderProgram(VERTEX_SHADER, FRAGMENT_SHADER, ShaderProgram::Flag_FromCode, stdUniforms, stdAttributes);
	glog(cachedLS->compilationLog());
	return cachedLS;
}

void ApplicationManager::oculusTick(double elapsed)
{
	tickLock.Lock();
	if (networkManager_)
		networkManager_->tick();

	if (player_ == false)
	{
		if (applicationStarted_ == false)
		{
			loadProperties();

			loadLuaFiles();
			skipFirstEnterFrame_ = true;

			applicationStarted_ = true;
			running_ = true;
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

	tickLock.Unlock();
}

#include "oculus/vr-room.h"
const float vrRoom_Loc[3]={40,-15,-50};
const float vrRoom_TV[3]={-45.57,24.777,4.175};
const float vrRoom_TVScale=0.042;
const float vrRoom_Scale=.08;
const float vrRoom_Floor=-15;
void ApplicationManager::oculusRender(float *vmat,float *pmat,int width, int height,bool room,bool screen,bool floor)
{
	application_->clearBuffers();
	application_->renderScene(1,vmat,pmat,[=](ShaderEngine *gfx,Matrix4 &xform)
			{
				gfx->setViewport(0, 0, width,height);
				if (room) {
					Matrix4 modelMat;
					modelMat.translate(vrRoom_Loc[0],vrRoom_Loc[1]-(floor?vrRoom_Floor:0),vrRoom_Loc[2]);
					modelMat.scale(vrRoom_Scale);
					gfx->clearColor(0.2,0.3,0.7,1);
					gfx->setModel(modelMat);
					 ShaderEngine::DepthStencil stencil=gfx->pushDepthStencil();
					 stencil.dTest=true;
					 gfx->setDepthStencil(stencil);
					 ShaderProgram *shp=getLightingShader(gfx);
					shp->setData(ShaderProgram::DataColor,ShaderProgram::DUBYTE,4,oculusRoomC,sizeof(oculusRoomC)/4,true,NULL);
					shp->setData(ShaderProgram::DataVertex,ShaderProgram::DFLOAT,3,oculusRoomV,sizeof(oculusRoomV)/(sizeof(float)*3),true,NULL);
					shp->setData(3,ShaderProgram::DFLOAT,3,oculusRoomN,sizeof(oculusRoomN)/(sizeof(float)*3),true,NULL);
					shp->drawElements(ShaderProgram::Triangles, sizeof(oculusRoomI)/sizeof(unsigned short), ShaderProgram::DUSHORT, oculusRoomI,true,NULL);
					gfx->popDepthStencil();
					if (screen) {
						xform.scale(vrRoom_TVScale,-vrRoom_TVScale,1);
						xform.translate(vrRoom_Loc[0]+vrRoom_TV[0],vrRoom_Loc[1]-(floor?vrRoom_Floor:0)+vrRoom_TV[1],vrRoom_Loc[2]+vrRoom_TV[2]);
						xform.scale(vrRoom_Scale);
					}
				}
			});

	drawIPs();
}


static void pushVector(lua_State *L,oculus::Vector v) {
	lua_newtable(L);
	lua_pushnumber(L,v.x); lua_rawseti(L,-2,1);
	lua_pushnumber(L,v.y); lua_rawseti(L,-2,2);
	lua_pushnumber(L,v.z); lua_rawseti(L,-2,3);
}

static void pushVector4(lua_State *L,oculus::Vector4 v) {
	lua_newtable(L);
	lua_pushnumber(L,v.x); lua_rawseti(L,-2,1);
	lua_pushnumber(L,v.y); lua_rawseti(L,-2,2);
	lua_pushnumber(L,v.z); lua_rawseti(L,-2,3);
	lua_pushnumber(L,v.w); lua_rawseti(L,-2,4);
}

void ApplicationManager::oculusInputEvent(oculus::Input &input) {
	lua_State *L=LuaDebugging::L;
	if (!L) return;
    lua_getglobal(L, "Oculus");
    lua_getfield(L,-1, "inputEventHandler");
    if (lua_isfunction(L,-1)) {
    	lua_newtable(L);
    	lua_pushinteger(L,input.deviceType);
		lua_setfield(L, -2, "deviceType");
    	lua_pushinteger(L,input.deviceId);
		lua_setfield(L, -2, "deviceId");
    	lua_pushinteger(L,input.batteryPercent);
		lua_setfield(L, -2, "batteryPercent");
    	lua_pushinteger(L,input.recenterCount);
		lua_setfield(L, -2, "recenterCount");

		//Pose
		lua_pushinteger(L,input.poseStatus);
		lua_setfield(L, -2,	"poseStatus");
		pushVector(L,input.pos);
		lua_setfield(L, -2, "position");
		pushVector4(L,input.rot);
		lua_setfield(L, -2, "rotation");
		pushVector(L,input.velPos);
		lua_setfield(L, -2, "linearVelocity");
		pushVector(L,input.velRot);
		lua_setfield(L, -2, "angularVelocity");
		pushVector(L,input.accPos);
		lua_setfield(L, -2, "linearAcceleration");
		pushVector(L,input.accRot);
		lua_setfield(L, -2, "angularAcceleration");
    	lua_pushinteger(L,input.caps);
		lua_setfield(L, -2, "caps");

		//Remote
		if (input.deviceType==4) {
			lua_pushinteger(L,input.buttons);
			lua_setfield(L, -2, "buttons");
			lua_pushnumber(L,input.stickX);
			lua_setfield(L, -2, "stickX");
			lua_pushnumber(L,input.stickY);
			lua_setfield(L, -2, "stickY");
			lua_pushnumber(L,input.gripTrigger);
			lua_setfield(L, -2, "gripTrigger");
			lua_pushnumber(L,input.indexTrigger);
			lua_setfield(L, -2, "indexTrigger");
		}

		//Hand
		if (input.deviceType==32) {
			lua_pushnumber(L,input.handScale);
			lua_setfield(L, -2, "handScale");
			lua_newtable(L);
			for (int k=0;k<24;k++) {
				pushVector4(L,input.handBone[k]);
				lua_rawseti(L,-2,k+1);
			}
			lua_setfield(L, -2, "handBone");
		}

	    lua_call(L, 1, 0);
	    lua_pop(L,1);
    }
    else
    	lua_pop(L,2);
}
#endif

void ApplicationManager::drawFrame()
{
	tickLock.Lock();
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

			tickLock.Unlock();
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
			running_ = true;
		}
		
		if (splashScreen_ && splashScreen_->isFinished())
		{
			application_->getApplication()->stage()->removeChild(splashScreen_);
			splashScreen_ = NULL;
			application_->getApplication()->setBackgroundColor(1, 1, 1, 1);

			loadLuaFiles();
			skipFirstEnterFrame_ = true;
			running_ = true;
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
	tickLock.Unlock();
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
	_OCULUS(onLuaReinit);
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

    	tickLock.Lock();
		GStatus status;
		application_->tick(&status);
    	tickLock.Unlock();
		if (status.error())
			luaError(status.errorString());
	}

	running_ = false;

	application_->deinitialize();
	application_->initialize();
	_OCULUS(onLuaReinit);
}

bool ApplicationManager::isRunning()
{
	return running_;
}

void ApplicationManager::drawIPs()
{
	if (player_ == true && running_ == false)
	{	
	    float canvasColor_[3];
	    float infoColor_[3]={0,0,0};
		int lWidth = application_->getLogicalWidth();
		int lHeight = application_->getLogicalHeight();
		drawInfoResolution(width_, height_, 100, lWidth, lHeight,
				true, canvasColor_, infoColor_, (int) application_->hardwareOrientation(), (int) application_->orientation(),
				1.0/application_->meanFrameTime_,1-(application_->meanFreeTime_/application_->meanFrameTime_));
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

void ApplicationManager::mouseWheel(int x,int y,int button,float amount)
{
	ginputp_mouseWheel(x,y,button,amount,0);
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

void ApplicationManager::textInput(const char *text,int ss,int se)
{
    gapplication_TextInputEvent *event = (gapplication_TextInputEvent*)gevent_CreateEventStruct1(
                                           sizeof(gapplication_TextInputEvent),
                                        offsetof(gapplication_TextInputEvent, text), text);
    event->selStart=ss;
    event->selEnd=se;

    gapplication_enqueueEvent(GAPPLICATION_TEXT_INPUT_EVENT, event, 1);

}

extern void gaudio_android_suspend(bool suspend);

void ApplicationManager::pause()
{
	gtexture_SaveRenderTargets();

    gapplication_enqueueEvent(GAPPLICATION_PAUSE_EVENT, NULL, 0);

	if (running_ == true)
	{
    	tickLock.Lock();
		GStatus status;
		application_->tick(&status);
    	tickLock.Unlock();
		if (status.error())
			luaError(status.errorString());
	}
	gaudio_android_suspend(true);
	paused_=true;
 }

void ApplicationManager::resume()
{
	paused_=false;
	gaudio_android_suspend(false);
    gapplication_enqueueEvent(GAPPLICATION_RESUME_EVENT, NULL, 0);

	if (running_ == true)
	{
    	tickLock.Lock();
		GStatus status;
		application_->tick(&status);
    	tickLock.Unlock();
		if (status.error())
			luaError(status.errorString());
	}
 }

void ApplicationManager::lowMemory()
{
    gapplication_enqueueEvent(GAPPLICATION_MEMORY_LOW_EVENT, NULL, 0);

	if (running_ == true)
	{
    	tickLock.Lock();
		GStatus status;
		application_->tick(&status);
    	tickLock.Unlock();
		if (status.error())
			luaError(status.errorString());
	}
}

void ApplicationManager::handleOpenUrl(const char *url)
{
	    gapplication_OpenUrlEvent *event = (gapplication_OpenUrlEvent*)gevent_CreateEventStruct1(
	                                           sizeof(gapplication_OpenUrlEvent),
	                                        offsetof(gapplication_OpenUrlEvent, url), url);

	    gapplication_enqueueEvent(GAPPLICATION_OPEN_URL_EVENT, event, 1);
}

void ApplicationManager::background()
{
    gapplication_enqueueEvent(GAPPLICATION_BACKGROUND_EVENT, NULL, 0);

	if (running_ == true)
	{
    	tickLock.Lock();
		GStatus status;
		application_->tick(&status);
    	tickLock.Unlock();
		if (status.error())
			luaError(status.errorString());
	}
 }

void ApplicationManager::foreground()
{
    gapplication_enqueueEvent(GAPPLICATION_FOREGROUND_EVENT, NULL, 0);

	if (running_ == true)
	{
    	tickLock.Lock();
		GStatus status;
		application_->tick(&status);
    	tickLock.Unlock();
		if (status.error())
			luaError(status.errorString());
	}
 }

void ApplicationManager::forceTick()
{
 	if (paused_&&running_)
	{
    	tickLock.Lock();
		GStatus status;
		application_->tick(&status);
    	tickLock.Unlock();
		if (status.error())
			luaError(status.errorString());
	}
}


static ApplicationManager *s_applicationManager = NULL;
void eventFlush()
{
    if (s_applicationManager)
    	s_applicationManager->forceTick();
}

#ifdef OCULUS
void oculus::doTick(double elapsed) {
	s_applicationManager->oculusTick(elapsed);
}
void oculus::doRender(float *vmat,float *pmat,int width, int height,bool room,bool screen,bool floor) {
	s_applicationManager->oculusRender(vmat,pmat,width,height,room,screen,floor);
}

static void oculus_callback_s(int type, void *event, void *udata)
{
	oculus::Input *input=(oculus::Input *)event;
	s_applicationManager->oculusInputEvent(*input);
}
static g_id oculusGid=g_NextId();
void oculus::doInputEvent(oculus::Input &input) {
	oculus::Input *_in=(oculus::Input *)malloc(sizeof(oculus::Input));
	*_in=input;
	gevent_EnqueueEvent(oculusGid, oculus_callback_s, 0, _in, 1, NULL);
}
#endif

extern "C" {

int GiderosOpenALConfig_sampleRate=44100;

void Java_com_giderosmobile_android_player_GiderosApplication_nativeOpenALSetup(JNIEnv *env, jclass cls, jint sampleRate)
{
	GiderosOpenALConfig_sampleRate=sampleRate;
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeCreate(JNIEnv *env, jclass cls, jboolean player, jobject activity)
{
	if (s_applicationManager != NULL)
		delete s_applicationManager;
	s_applicationManager = new ApplicationManager(env, player != 0);
	_OCULUS(onCreate,env,activity,s_applicationManager->getApplication());
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeDestroy(JNIEnv *env, jclass cls)
{
	_OCULUS(onDestroy,env);
	delete s_applicationManager;
	s_applicationManager = NULL;
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeSurfaceCreated(JNIEnv *env, jclass cls, jobject surface)
{
#ifdef OCULUS
	_OCULUS(onSurfaceCreated,env,surface);
#else
	s_applicationManager->surfaceCreated();
#endif
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeSurfaceChanged(JNIEnv *env, jclass cls, jint width, jint height, jint rotation, jobject surface)
{
#ifdef OCULUS
	_OCULUS(onSurfaceChanged,env,surface);
#endif
	s_applicationManager->surfaceChanged(width, height, rotation);
}

void Java_com_giderosmobile_android_player_GiderosApplication_nativeSurfaceDestroyed(JNIEnv *env, jclass cls)
{
	_OCULUS(onSurfaceDestroyed);
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

void Java_com_giderosmobile_android_player_GiderosApplication_nativeMouseWheel(JNIEnv* env, jobject thiz, jint x,jint y,jint button,jfloat amount)
{
	s_applicationManager->mouseWheel(x,y,button,amount);
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

void Java_com_giderosmobile_android_player_GiderosApplication_nativeTextInput(JNIEnv* env, jclass cls, jstring text,jint ss,jint se)
{
	const char* sBytes = env->GetStringUTFChars(text, NULL);
	if (ss>0) {
		const char *r=sBytes;
		while (ss&&(*r)) {
			int cr=(*r)&0xFF;
			if ((cr<0x80)||(cr>=0xC0))
				ss--;
			r++;
		}
		while (((*r)&0xC0)==0x80) r++;
		ss=r-sBytes;
	}
	if (se>0) {
		const char *r=sBytes;
		while (se&&(*r)) {
			int cr=(*r)&0xFF;
			if ((cr<0x80)||(cr>=0xC0))
				se--;
			r++;
		}
		while (((*r)&0xC0)==0x80) r++;
		se=r-sBytes;
	}

	s_applicationManager->textInput(sBytes,ss,se);
	env->ReleaseStringUTFChars(text, sBytes);
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

void Java_com_giderosmobile_android_player_GiderosApplication_nativeHandleOpenUrl(JNIEnv* env, jclass cls, jstring url)
{
	const char* sBytes = env->GetStringUTFChars(url, NULL);
	s_applicationManager->handleOpenUrl(sBytes);
	env->ReleaseStringUTFChars(url, sBytes);
}

void Java_com_giderosmobile_android_player_GiderosApplication_oculusPause(JNIEnv* env, jclass cls)
{
	_OCULUS(onPause);
}

void Java_com_giderosmobile_android_player_GiderosApplication_oculusResume(JNIEnv* env, jclass cls)
{
	_OCULUS(onResume);
}

void Java_com_giderosmobile_android_player_GiderosApplication_oculusStop(JNIEnv* env, jclass cls)
{
	_OCULUS(onStop);
}

void Java_com_giderosmobile_android_player_GiderosApplication_oculusStart(JNIEnv* env, jclass cls)
{
	_OCULUS(onStart);
}

void Java_com_giderosmobile_android_player_GiderosApplication_oculusRunThread(JNIEnv* env, jclass cls)
{
	_OCULUS(runThread);
}

void Java_com_giderosmobile_android_player_GiderosApplication_oculusPostCreate(JNIEnv* env, jclass cls)
{
	_OCULUS(postCreate);
}

}


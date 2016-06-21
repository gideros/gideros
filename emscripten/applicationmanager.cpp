//#include <jni.h>
//#include <android/log.h>
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
#include <limits.h>

#include <platform.h>

#include <gtexture.h>

#include <gstdio.h>

#include <splashscreen.h>
#include <application.h>
#include <stage.h>

#include <gpath.h>
#include <gvfs-native.h>

#include <pystring.h>

//#include <javanativebridge.h>

#include <GLES/gl.h>

#include <glog.h>

#include <keycode.h>

#include <binder.h>

#include <gui.h>
#include <ginput.h>
#include <ginput-js.h>
#include <gevent.h>
#include <ggeolocation.h>

#include <glog.h>

#include <gtexture.h>

#include <gaudio.h>

#include <gapplication.h>
#include <gapplication-js.h>
#include <applicationmanager.h>
#include <emscripten.h>

extern "C" {
int g_getFps();
void g_setFps(int fps);
}
void drawInfo();
void refreshLocalIPs();
void g_exit();

static void printJS(const char *str, int len, void *data) {
 if (len>=0)
 {
  char *m=(char *)malloc(len+1);
  memcpy(m,str,len);
  m[len]=0;
  //printf(m);
  EM_ASM_({ Module.luaPrint(Pointer_stringify($0)) },m);
  free(m);
 }
 else
 {
  //printf(str);
  EM_ASM_({ Module.luaPrint(Pointer_stringify($0)) },str);
 }
}

static volatile const char* licenseKey_ = "9852564f4728e0c11e34ca3eb5fe20b2";
//-----------------------------------------01234567890123456------------------

const char *codeKey_ = "312e68c04c6fd22922b5b232ea6fb3e1"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
static const char *assetsKey_ = "312e68c04c6fd22922b5b232ea6fb3e2"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
		"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";


static void _mkdir(const char *dir) {
        char tmp[PATH_MAX];
                char *p = NULL;
                        size_t len;
                        
                                snprintf(tmp, sizeof(tmp),"%s",dir);
                                        len = strlen(tmp);
                                                if(tmp[len - 1] == '/')
                                                                tmp[len - 1] = 0;
                                                                        for(p = tmp + 1; *p; p++)
                                                                                        if(*p == '/') {
                                                                                                                *p = 0;
                                                                                                                                        mkdir(tmp, S_IRWXU);
                                                                                                                                                                *p = '/';
                                                                                                                                                                                }
                                                                                                                                                                                        mkdir(tmp, S_IRWXU);
                                                                                                                                                                                        }
                                                                                                                                                                                        
ApplicationManager::ApplicationManager(bool player,const char *appname,const char *urlpath) {
	player_ = player;
	appName=appname;
	appPath=urlpath;

	// gpath & gvfs
	gpath_init();
	gpath_addDrivePrefix(0, "|R|");
	gpath_addDrivePrefix(0, "|r|");
	gpath_addDrivePrefix(1, "|D|");
	gpath_addDrivePrefix(1, "|d|");
	gpath_addDrivePrefix(2, "|T|");
	gpath_addDrivePrefix(2, "|t|");

	bool hasDocuments=EM_ASM_INT_V({ return FS.documentsOk; });
	gpath_setDriveFlags(0, GPATH_RO);
	gpath_setDriveFlags(1, (hasDocuments?GPATH_RW:GPATH_RO) | GPATH_REAL);
	gpath_setDriveFlags(2, GPATH_RW | GPATH_REAL);

	gpath_setAbsolutePathFlags(GPATH_RW | GPATH_REAL);

	gpath_setDefaultDrive(0);

	gvfs_init();
	//gvfs_setPlayerModeEnabled(player ? 1 : 0);
	gvfs_setCodeKey(codeKey_ + 32);
	gvfs_setAssetsKey(assetsKey_ + 32);

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

	// application
	networkManager_=NULL;
	application_ = new LuaApplication;
	application_->setPlayerMode(player_);
/*	if (player_)
		application_->setPrintFunc(NetworkManager::printToServer_s,
				networkManager_);
        else*/
		application_->setPrintFunc(printJS,NULL);
        
	application_->enableExceptions();

	Binder::disableTypeChecking();

	init_ = false;
	running_ = false;

	width_ = 0;
	height_ = 0;

	//splashScreen_ = NULL;

	nframe_ = 0;

	applicationStarted_ = false;

	skipFirstEnterFrame_ = false;
}

ApplicationManager::~ApplicationManager() {
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

void ApplicationManager::luaError(const char *error) {
  EM_ASM_({ Module.luaError(Pointer_stringify($0)) },error);
  if (player_)
  {
     running_ = false;
     application_->deinitialize();
     application_->initialize();       
  }
/*  else
   g_exit(); */
}

void ApplicationManager::surfaceCreated() {
	if (!init_) {
		init_ = true;
		application_->initialize();
	} else {
		gtexture_reloadTextures();
		gtexture_RestoreRenderTargets();
		gtexture_RestoreTempTextures();
	}
}

void ApplicationManager::surfaceChanged(int width, int height, int rotation) {
	if (player_ == true)
		refreshLocalIPs();

/*	if (width > height) {
		width_ = height;
		height_ = width;
	} else {
		width_ = width;
		height_ = height;
	}*/

	width_=width;
	height_=height;
	updateHardwareOrientation();
	application_->setResolution(width_, height_);
	if (ShaderEngine::Engine)
	 ShaderEngine::Engine->reset(true);
 
/*	switch (rotation) {
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
*/
        Event event(Event::APPLICATION_RESIZE);
        GStatus status;
        application_->broadcastEvent(&event, &status);                        
}

void ApplicationManager::updateHardwareOrientation() {
/*	Orientation orientation = application_->orientation();

	bool b1 = orientation == ePortrait || orientation == ePortraitUpsideDown;
	bool b2 = deviceOrientation_ == ePortrait
			|| deviceOrientation_ == ePortraitUpsideDown;

	if (b1 != b2)
		hardwareOrientation_ = deviceOrientation_;
	else
		hardwareOrientation_ = orientation;
*/
        hardwareOrientation_ = eFixed;
	application_->setHardwareOrientation(hardwareOrientation_);
}

static bool canvasShown=false;
void ApplicationManager::drawFrame() {
	if (networkManager_)
		networkManager_->tick();

	// if we're in app mode, skip the first 10 frames as black screen
	// because a pattern like surfaceChanged(a,b), drawFrame, drawFrame, drawFrame, ..., surfaceChanged(b,a) is possible
	// and we want to execute lua files after getting the final device resolution
	if (player_ == false) {
		if (nframe_++ < 10) {
			glClearColor(0, 0, 0, 1);
			glClear (GL_COLOR_BUFFER_BIT);

			return;
		}
	}

	if (player_ == false) {
		if (applicationStarted_ == false) {
			if (!appName.empty())
				play(appName.c_str());
			else
			{
				loadProperties();
				loadLuaFiles();
			}
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

		/*		if (splashScreen_ && splashScreen_->isFinished())
		 {
		 application_->getApplication()->stage()->removeChild(splashScreen_);
		 splashScreen_ = NULL;
		 application_->getApplication()->setBackgroundColor(1, 1, 1);

		 loadLuaFiles();
		 skipFirstEnterFrame_ = true;
		 }*/
	}

	if (skipFirstEnterFrame_ == true) {
		skipFirstEnterFrame_ = false;
	} else {
		GStatus status;
		application_->enterFrame(&status);
		if (status.error())
			luaError(status.errorString());
          gaudio_AdvanceStreamBuffers();		  
	}

	application_->clearBuffers();
	application_->renderScene(1);
	drawIPs();
	
	if (!canvasShown)
	{
	 canvasShown=true;
	 EM_ASM(Module.setStatus("Running"));
	}
}

void ApplicationManager::loadProperties() {
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
	for (int i = 0; i < scaleCount; ++i) {
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
	application_->setOrientation((Orientation) properties_.orientation);
	updateHardwareOrientation();
	application_->getApplication()->setDeviceOrientation(deviceOrientation_);
	application_->setLogicalDimensions(properties_.logicalWidth,
			properties_.logicalHeight);
	application_->setLogicalScaleMode((LogicalScaleMode) properties_.scaleMode);
	application_->setImageScales(properties_.imageScales);

	g_setFps(properties_.fps);

	ginput_setMouseToTouchEnabled(properties_.mouseToTouch);
	ginput_setTouchToMouseEnabled(properties_.touchToMouse);
	ginput_setMouseTouchOrder(properties_.mouseTouchOrder);
}

void ApplicationManager::loadLuaFiles() {
	std::vector<std::string> luafiles;

	G_FILE* fis = g_fopen("luafiles.txt", "rt");

	if (fis) {
		char line[1024];
		while (true) {
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
	for (size_t i = 0; i < luafiles.size(); ++i) {
		application_->loadFile(luafiles[i].c_str(), &status);
		if (status.error())
			break;
	}

	if (!status.error()) {
		gapplication_enqueueEvent(GAPPLICATION_START_EVENT, NULL, 0);
		application_->tick(&status);
	}

	if (status.error())
		luaError(status.errorString());
}

void ApplicationManager::setDirectories(const char *externalDir,
		const char *internalDir, const char *cacheDir) {
	externalDir_ = externalDir;
	internalDir_ = internalDir;
	cacheDir_ = cacheDir;
}

void ApplicationManager::setFileSystem(const char *fileSystem) {
	std::vector<std::string> result;
	pystring::split(fileSystem, result, "|");

//	gvfs_setZipFiles(result[0].c_str(), result[1].c_str(), result[2].c_str());

	for (size_t i = 3; i < result.size(); i += 4) {
		gvfs_addFile(result[i].c_str(), atoi(result[i + 1].c_str()),
				atoi(result[i + 2].c_str()), atoi(result[i + 3].c_str()));
		glog_v("%s %d %d %d", result[i].c_str(), atoi(result[i + 1].c_str()),
				atoi(result[i + 2].c_str()), atoi(result[i + 3].c_str()));
	}

	setDocumentsDirectory(internalDir_.c_str());
	setTemporaryDirectory(cacheDir_.c_str());
}

void ApplicationManager::setOpenProject(const char* project) {
	networkManager_->openProject_ = project;
}


#include "netendian.h"
void ApplicationManager::play(const char *gapp) {
	FILE *fd=fopen(gapp,"rb");
	if (!fd) return; //No file/not openable
	fseek(fd,0,SEEK_END);
	long pksz=ftell(fd);
	if (pksz < 16)
		return; //Invalid file size
	struct {
		uint32_t flistOffset;
		uint32_t version;
		char signature[8];
	} PACKED tlr;
	fseek(fd,pksz - 16,SEEK_SET);
	fread(&tlr,1,16,fd);
	tlr.version = BIGENDIAN4(tlr.version);
	tlr.flistOffset = BIGENDIAN4(tlr.flistOffset);
	if ((!strncmp(tlr.signature, "GiDeRoS", 7)) && (tlr.version == 0)) {
		glog_v("GAPP-ARCH: %s", gapp);
		gvfs_setZipFile(gapp);
		char *buffer = (char *) malloc(pksz - tlr.flistOffset);
		fseek(fd,tlr.flistOffset,SEEK_SET);
		fread(buffer, 1,pksz - tlr.flistOffset,fd);
		int offset = 0;
		while (offset < (pksz - tlr.flistOffset)) {
			int plen = strlen(buffer + offset);
			if (!plen)
				break; //End of list
			uint32_t foffset=PBULONG(buffer + offset + plen + 1);
			uint32_t fsize=PBULONG(buffer + offset + plen + 1+sizeof(uint32_t));
			const char *norm = gpath_normalizeArchivePath(buffer + offset);
			gvfs_addFile(norm, 0, foffset, fsize);
			//glog_d("GAPP-FILE: %s,%d,%d", norm, foffset, fsize);
			offset += (plen + 1 + 2 * sizeof(uint32_t));
		}
		free(buffer);
	} else
		glog_w("GAPP: Invalid signature/version");
	fclose(fd);

	if (running_ == true) {
		Event event(Event::APPLICATION_EXIT);
		GStatus status;
		application_->broadcastEvent(&event, &status);
		running_ = false;
	}

	std::string gappfile=gapp;
	std::string projectName=gappfile.substr(0,gappfile.find_last_of('.')-1);


	const char* documentsDirectory;
	const char* temporaryDirectory;

	std::string dir = "/";

	std::string documents = dir + "documents"+appPath;
	std::string temporary = dir + "temporary";
	_mkdir(documents.c_str());
	_mkdir(temporary.c_str());

	glog_v("documents: %s", documents.c_str());
	glog_v("temporary: %s", temporary.c_str());

	mkdir(documents.c_str(), 0755);
	mkdir(temporary.c_str(), 0755);

	setDocumentsDirectory(documents.c_str());
	setTemporaryDirectory(temporary.c_str());
	setResourceDirectory("");

	loadProperties();
	loadLuaFiles();
}

void ApplicationManager::openProject(const char* project) {

	//setting project name
	setProjectName(project);
	playFiles("../properties.bin","../luafiles.txt");
}

void ApplicationManager::playFiles(const char* pfile,const char *lfile) {


	//setting properties
	const char* propfilename = g_pathForFile(pfile);
	FILE* fis_prop = fopen(propfilename, "rb");

	const char* luafilename = g_pathForFile(lfile);
	FILE* fis_lua = fopen(luafilename, "rb");

	if (fis_prop != NULL && fis_lua != NULL) {

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
		for (int i = 0; i < scaleCount; ++i) {
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

		const char* luafilename = g_pathForFile(lfile);
		FILE* fis_lua = fopen(luafilename, "rb");

		fseek(fis_lua, 0, SEEK_END);
		len = ftell(fis_lua);
		fseek(fis_lua, 0, SEEK_SET);

		std::vector<char> buf_lua(len);
		fread(&buf_lua[0], 1, len, fis_lua);
		fclose(fis_lua);

		ByteBuffer buffer2(&buf_lua[0], buf_lua.size());

		buffer2 >> chr;

		while (buffer2.eob() == false) {
			std::string str;
			buffer2 >> str;
			luafiles.push_back(str);
		}

		play(luafiles);
	}
}

void ApplicationManager::play(const std::vector<std::string>& luafiles) {
	running_ = true;

	application_->deinitialize();
	application_->initialize();
	application_->setResolution(width_, height_);
	application_->setOrientation((Orientation) properties_.orientation);
	updateHardwareOrientation();
	application_->getApplication()->setDeviceOrientation(deviceOrientation_);
	application_->setLogicalDimensions(properties_.logicalWidth,
			properties_.logicalHeight);
	application_->setLogicalScaleMode((LogicalScaleMode) properties_.scaleMode);
	application_->setImageScales(properties_.imageScales);

	g_setFps(properties_.fps);

	ginput_setMouseToTouchEnabled(properties_.mouseToTouch);
	ginput_setTouchToMouseEnabled(properties_.touchToMouse);
	ginput_setMouseTouchOrder(properties_.mouseTouchOrder);

	GStatus status;
	for (std::size_t i = 0; i < luafiles.size(); ++i) {
		application_->loadFile(luafiles[i].c_str(), &status);
		if (status.error())
			break;
	}

	if (!status.error()) {
		gapplication_enqueueEvent(GAPPLICATION_START_EVENT, NULL, 0);
		application_->tick(&status);
	}

	if (status.error())
		luaError(status.errorString());
}

void ApplicationManager::stop() {
	if (running_ == true) {
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

bool ApplicationManager::isRunning() {
	return running_;
}

void ApplicationManager::drawIPs() {
	if (player_ == true && running_ == false) {
		drawInfo();
	}
}

void ApplicationManager::setProjectName(const char *projectName) {
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

void ApplicationManager::setProjectProperties(
		const ProjectProperties &properties) {
	properties_ = properties;
}
/*
 void ApplicationManager::touchesBegin(int size, int *id, int *x, int *y, int actionIndex)
 {
 ginputp_touchBegin(size, id, x, y, actionIndex);
 }

 void ApplicationManager::touchesMove(int size, int *id, int *x, int *y)
 {
 ginputp_touchesMove(size, id, x, y);
 }

 void ApplicationManager::touchesEnd(int size, int *id, int *x, int *y, int actionIndex)
 {
 ginputp_touchEnd(size, id, x, y, actionIndex);
 }

 void ApplicationManager::touchesCancel(int size, int *id, int *x, int *y)
 {
 ginputp_touchesCancel(size, id, x, y);
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
 */
void ApplicationManager::pause() {
	gtexture_SaveRenderTargets();

	gapplication_enqueueEvent(GAPPLICATION_PAUSE_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
}

void ApplicationManager::resume() {
	gapplication_enqueueEvent(GAPPLICATION_RESUME_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
}

void ApplicationManager::lowMemory() {
	gapplication_enqueueEvent(GAPPLICATION_MEMORY_LOW_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
}

void ApplicationManager::background() {
	gapplication_enqueueEvent(GAPPLICATION_BACKGROUND_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
}

void ApplicationManager::foreground() {
	gapplication_enqueueEvent(GAPPLICATION_FOREGROUND_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
}


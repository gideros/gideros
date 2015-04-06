#include "giderosapi.h"
#include "pch.h"
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stack>
#include <string>
#include <binder.h>
#include <xaudio2.h>
//#include <libnetwork.h>
#include "ginput-winrt.h"
#include "luaapplication.h"
#include "platform.h"
#include "refptr.h"
#include <bytebuffer.h>
#include <event.h>
#include <application.h>
#include <gui.h>
#include <keycode.h>
#include <gevent.h>
#include <ginput.h>
#include <glog.h>
#include "gpath.h"
#include <gfile.h>
#include <gfile_p.h>
#include "gvfs-native.h"
#include "ggeolocation.h"
#include "gapplication.h"
#include "gaudio.h"
#include "ghttp.h"
#include <gapplication.h>
#include <gapplication-winrt.h>
#include "dxcompat.hpp"
#include "dxglobals.h"

extern "C" {
	void g_setFps(int);
	int g_getFps();
}
void drawInfo();
void refreshLocalIPs();
void g_exit();

static volatile const char* licenseKey_ = "9852564f4728e0c11e34ca3eb5fe20b2";
//-----------------------------------------01234567890123456------------------

static const char *codeKey_ = "312e68c04c6fd22922b5b232ea6fb3e1\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
static const char *assetsKey_ = "312e68c04c6fd22922b5b232ea6fb3e2\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";

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

		int pos = 0;
		buffer[pos] = 4;
		pos += 1;
		strcpy(buffer + pos, str);
		pos += strlen(str) + 1;

		//server_->sendData(buffer, size);

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
	//Server *server_;
	std::string resourceDirectory_;
};


class ApplicationManager
{
public:
	ApplicationManager(int width, int height, bool player, const wchar_t* resourcePath, const wchar_t* docsPath);
	~ApplicationManager();

	void drawFirstFrame();

	void luaError(const char *msg);

	void drawFrame();

	void openProject(const char* project);
	void setOpenProject(const char* project);

	void play(const std::vector<std::string>& luafiles);
	void stop();
	void setProjectName(const char *projectName);
	void setProjectProperties(const ProjectProperties &properties);
	bool isRunning();

	void didReceiveMemoryWarning();

	void suspend();
	void resume();
	void exitRenderLoop();
	void foreground();
	void background();
	

private:
	void loadProperties();
	void loadLuaFiles();
	void drawIPs();
	int convertKeyCode(int keyCode);

private:
	bool player_;
	LuaApplication *application_;
	NetworkManager *networkManager_;
	const wchar_t* resourcePath_;
	const wchar_t* docsPath_;

	bool running_;

	int width_, height_;

	ProjectProperties properties_;

	Orientation hardwareOrientation_;

	Orientation deviceOrientation_;

	bool luaFilesLoaded_;

	int nframe_;
};


NetworkManager::NetworkManager(ApplicationManager* application)
{
	application_ = application;
	//server_ = new Server(15000);
}

NetworkManager::~NetworkManager()
{
	//delete server_;
}

void NetworkManager::tick()
{
	/*int dataTotal = 0;

	while (true)
	{
		if (!openProject_.empty()){
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
	}*/
}

void NetworkManager::createFolder(const std::vector<char>& data)
{
	std::string folderName = &data[1];
	CreateDirectory((LPCWSTR)g_pathForFile(folderName.c_str()), NULL);
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

	//server_->sendData(buffer.data(), buffer.size());
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


ApplicationManager::ApplicationManager(int width, int height, bool player, const wchar_t* resourcePath, const wchar_t* docsPath)
{
	width_ = width;
	height_ = height;
	player_ = player;
	resourcePath_ = resourcePath;
	docsPath_ = docsPath;

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

	Binder::disableTypeChecking();

	hardwareOrientation_ = ePortrait;

	deviceOrientation_ = ePortrait;

	running_ = false;

	nframe_ = 0;

	if (player_ == false)
	{
		const wchar_t *installedLocation = resourcePath_;

		char fileStem[MAX_PATH];
		wcstombs(fileStem, installedLocation, MAX_PATH);
		strcat(fileStem, "\\assets\\");
		setResourceDirectory(fileStem);

		gpath_setDrivePath(0, fileStem);

		const wchar_t *docs = docsPath_;

		char docsPath[MAX_PATH];
		wcstombs(docsPath, docs, MAX_PATH);
		strcat(docsPath, "\\");

		setDocumentsDirectory(docsPath);
		setTemporaryDirectory(docsPath);

		gpath_setDrivePath(1, docsPath);

		loadProperties();

		// Gideros has became open source and free, because this, there's no more splash art
		loadLuaFiles();

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

void ApplicationManager::drawFirstFrame()
{

	application_->clearBuffers();
	application_->renderScene(1);
	drawIPs();
}

void ApplicationManager::drawFrame()
{

	nframe_++;

	if (networkManager_)
		networkManager_->tick();

	application_->clearBuffers();

	if (application_->isErrorSet())
		luaError(application_->getError());

	GStatus status;
	application_->enterFrame(&status);
	if (status.error())
		luaError(status.errorString());

	application_->renderScene(1);
	drawIPs();
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

	if (fis_prop != NULL && fis_lua != NULL){

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

void ApplicationManager::setProjectName(const char *projectName)
{
	glog_v("setProjectName: %s", projectName);
	std::wstring ws(docsPath_);
	std::string dir = std::string(ws.begin(), ws.end());

	if (dir[dir.size() - 1] != '/')
		dir += "/";

	dir += "gideros";

	CreateDirectory((LPCWSTR)dir.c_str(), NULL);

	dir += "/";

	dir += projectName;

	CreateDirectory((LPCWSTR)dir.c_str(), NULL);

	dir += "/";

	std::string md5filename_ = dir + "md5.txt";

	std::string documents = dir + "documents";
	std::string temporary = dir + "temporary";
	std::string resource = dir + "resource";

	glog_v("documents: %s", documents.c_str());
	glog_v("temporary: %s", temporary.c_str());
	glog_v("resource: %s", resource.c_str());

	CreateDirectory((LPCWSTR)documents.c_str(), NULL);
	CreateDirectory((LPCWSTR)temporary.c_str(), NULL);
	CreateDirectory((LPCWSTR)resource.c_str(), NULL);

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
		g_exit();
	}
}

void ApplicationManager::play(const std::vector<std::string>& luafiles)
{
	running_ = true;

	application_->deinitialize();
	application_->initialize();
	float contentScaleFactor = 1;
	application_->setResolution(width_ * contentScaleFactor, height_ * contentScaleFactor);
	application_->setHardwareOrientation(hardwareOrientation_);
	application_->getApplication()->setDeviceOrientation(deviceOrientation_);
	application_->setOrientation((Orientation)properties_.orientation);
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

	float contentScaleFactor = 1;

	application_->setResolution(width_ * contentScaleFactor, height_ * contentScaleFactor);
	application_->setHardwareOrientation(hardwareOrientation_);
	application_->getApplication()->setDeviceOrientation(deviceOrientation_);
	application_->setOrientation((Orientation)properties_.orientation);
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

void ApplicationManager::drawIPs()
{
	if (player_ == true && running_ == false)
	{
		drawInfo();
	}
}

void ApplicationManager::didReceiveMemoryWarning()
{
	gapplication_enqueueEvent(GAPPLICATION_MEMORY_LOW_EVENT, NULL, 0);

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

void ApplicationManager::background()
{
	gapplication_enqueueEvent(GAPPLICATION_BACKGROUND_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
}

void ApplicationManager::suspend()
{
	gapplication_enqueueEvent(GAPPLICATION_PAUSE_EVENT, NULL, 0);
	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
}

void ApplicationManager::resume()
{
	gapplication_enqueueEvent(GAPPLICATION_RESUME_EVENT, NULL, 0);
	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());

}

void ApplicationManager::exitRenderLoop()
{
	gapplication_enqueueEvent(GAPPLICATION_EXIT_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
}



static ApplicationManager *s_manager = NULL;

extern "C" {

	void gdr_initialize(int width, int height, bool player, const wchar_t* resourcePath, const wchar_t* docsPath)
	{
		s_manager = new ApplicationManager(width, height, player, resourcePath, docsPath);
	}

	void gdr_drawFrame()
	{
		s_manager->drawFrame();
	}

	void gdr_exitGameLoop()
	{
		s_manager->exitRenderLoop();
	}

	void gdr_openProject(const char* project)
	{

		s_manager->setOpenProject(project);
	}

	void gdr_suspend(){
		s_manager->suspend();
	}

	void gdr_resume(){
		s_manager->resume();
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

	void gdr_didReceiveMemoryWarning()
	{
		s_manager->didReceiveMemoryWarning();
	}

	bool gdr_isRunning()
	{
		return s_manager->isRunning();
	}

}
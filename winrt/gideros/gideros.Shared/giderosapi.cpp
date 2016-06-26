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
#include <direct.h>
#include <binder.h>
#include <xaudio2.h>
#include <libnetwork.h>
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
#include <windows.ui.xaml.media.dxinterop.h>

using namespace Platform;
using namespace Windows::UI::Core;
using namespace Windows::Graphics::Display;

static ULONGLONG next_game_tick;

/*
* Mutex Functions
*/

#include "pthread.h"
#include "dx11Shaders.h"
//#define PTW32_DLLPORT
//#define PTW32_CDECL

#ifdef WINSTORE
ComPtr<IDXGIDevice3> dxgiDevice;
#endif

int PTW32_CDECL pthread_mutex_init(pthread_mutex_t * mutex,
	const pthread_mutexattr_t * attr)
{
	CRITICAL_SECTION *c = (CRITICAL_SECTION*) malloc(sizeof(CRITICAL_SECTION));

	InitializeCriticalSectionEx(c,0,0);
	*((LPCRITICAL_SECTION *)mutex) = c;
	return 0;
}

int PTW32_CDECL pthread_mutex_destroy(pthread_mutex_t * mutex)
{
	free(*((LPCRITICAL_SECTION *)mutex));
	*mutex = NULL;
	return 0;
}


int PTW32_CDECL pthread_mutex_lock(pthread_mutex_t * mutex)
{
	if (*mutex == PTHREAD_MUTEX_INITIALIZER)
		pthread_mutex_init(mutex, NULL);
	EnterCriticalSection(*((LPCRITICAL_SECTION *)mutex));
	return 0;
}


int PTW32_CDECL pthread_mutex_timedlock(pthread_mutex_t * mutex,
	const struct timespec *abstime)
{
	return 0;
}


int PTW32_CDECL pthread_mutex_trylock(pthread_mutex_t * mutex)
{
	if (*mutex == PTHREAD_MUTEX_INITIALIZER)
		pthread_mutex_init(mutex, NULL);
	return TryEnterCriticalSection(*((LPCRITICAL_SECTION *)mutex)) ? 0 : -1;
}

int PTW32_CDECL pthread_mutex_unlock(pthread_mutex_t * mutex)
{
	LeaveCriticalSection(*((LPCRITICAL_SECTION *)mutex));
	return 0;
}

int PTW32_CDECL pthread_mutex_consistent(pthread_mutex_t * mutex)
{
	return 0;
}

int PTW32_CDECL pthread_create(pthread_t * tid,
	const pthread_attr_t * attr,
	void *(PTW32_CDECL *start) (void *),
	void *arg)
{
	return 0;
}

int PTW32_CDECL pthread_join(pthread_t thread,
	void **value_ptr)
{
	return 0;
}

IXAudio2 *g_audioengine;
IXAudio2MasteringVoice *g_masteringvoice;
IXAudio2SourceVoice* g_source;

static void printFunc(const char *str, int len, void *data)
{
	std::string s(str);
	std::wstring wsTmp(s.begin(), s.end());
	OutputDebugString(wsTmp.c_str());
}


//######################################################################
// this function initializes and prepares Direct3D for use
void InitD3D(bool useXaml, CoreWindow^ Window, Windows::UI::Xaml::Controls::SwapChainPanel ^swapChainPanel, int width, int height)
{

	// ----------------------------------------------------------------------
	// Create swap chain, get g_dev, g_devcon
	// ----------------------------------------------------------------------

	ComPtr<ID3D11Device> dev11;
	ComPtr<ID3D11DeviceContext> devcon11;

	D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&dev11,
		nullptr,
		&devcon11);

	// Convert the pointers from Direct3D 11 to Direct3D 11.1
	dev11.As(&g_dev);
	devcon11.As(&g_devcon);

	// Obtain the DXGI factory
	// ComPtr<IDXGIDevice1> dxgiDevice;
	g_dev.As(&dxgiDevice);
	ComPtr<IDXGIAdapter> dxgiAdapter;
	dxgiDevice->GetAdapter(&dxgiAdapter);
	ComPtr<IDXGIFactory2> dxgiFactory;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), &dxgiFactory);

	// create a struct to hold information about the swap chain    
	DXGI_SWAP_CHAIN_DESC1 scd;
	// clear out the struct for use    
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC1));

	// fill the swap chain description struct    
	scd.Width = useXaml?width:0;
	scd.Height = useXaml?height:0;
	scd.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scd.Stereo = false;
	scd.SampleDesc.Count = 1;                               	// DISABLE ANTI-ALIASING
	scd.SampleDesc.Quality = 0;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      	// how swap chain is to be used    
	scd.BufferCount = 2;                                    	// one back buffer for WP8 (Windows 8: 2)
	//scd.Scaling = DXGI_SCALING_STRETCH;                       // WP8 (Windows 8: not set)
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;                // WP8 (Windows 8: DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL)
	scd.Flags = 0;

	if (useXaml){
		dxgiFactory->CreateSwapChainForComposition(
			g_dev.Get(),
			&scd,
			nullptr,
			&g_swapchain
			);

		Microsoft::WRL::ComPtr<ISwapChainPanelNative> swapChainNative;
		IInspectable* panelInspectable = (IInspectable*) reinterpret_cast<IInspectable*>(swapChainPanel);
		panelInspectable->QueryInterface(__uuidof(ISwapChainPanelNative), (void **)&swapChainNative);

		swapChainNative->SetSwapChain(g_swapchain.Get());  // ties the panel to our DirectX swapchain
	}
	else {

		dxgiFactory->CreateSwapChainForCoreWindow(
			g_dev.Get(),
			reinterpret_cast<IUnknown*>(Window),
			&scd,
			nullptr,
			&g_swapchain);
	}

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	float scaley;

//	if (useXaml){
//		scaley = 1;
//	}
//	else {
		DisplayInformation ^dinfo = DisplayInformation::GetForCurrentView();
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
		scaley = dinfo->RawPixelsPerViewPixel; // Windows phone
#else
		scaley = ((int)dinfo->ResolutionScale)*0.01f*dinfo->LogicalDpi/ 96.0f;   // Windows 8 PC
#endif
//    }

	float basex = 0;
	float basey = 0;
	float windoww = width*scaley;
	float windowh = height*scaley;

	viewport.TopLeftX = basex;
	viewport.TopLeftY = basey;
	viewport.Width = windoww;  // Direct3D needs actual pixels
	viewport.Height = windowh;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0;

	g_devcon->RSSetViewports(1, &viewport);
}

// ######################################################################
// this is the function that cleans up Direct3D and COM
void CleanD3D()
{
//XXX	pBackBuffer->Release();
	//  g_swapchain->Release();    
	//  g_dev->Release();    
	//  g_devcon->Release();
}


// ######################################################################
void InitXAudio2()
{
	XAudio2Create(&g_audioengine);
	HRESULT hr = g_audioengine->CreateMasteringVoice(&g_masteringvoice);
}

// ######################################################################
void CleanXAudio2()
{
	g_audioengine->Release();
}

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
	ApplicationManager(bool useXaml, CoreWindow^ Window, Windows::UI::Xaml::Controls::SwapChainPanel ^swapChainPanel, 
		int width, int height, bool player, const wchar_t* resourcePath, const wchar_t* docsPath, const wchar_t* tempPath);
	~ApplicationManager();

	void getStdCoords(float xp, float yp, float &x, float &y);

	void drawFirstFrame();

	void luaError(const char *msg);

	void drawFrame(bool useXaml);

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
	void resize(int width, int height);

	Windows::UI::Xaml::Controls::SwapChainPanel^ getRoot();

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
	const wchar_t* tempPath_;

	float contentScaleFactor;

	bool running_;

	int width_, height_;

	ProjectProperties properties_;

	Orientation hardwareOrientation_;

	Orientation deviceOrientation_;

	bool luaFilesLoaded_;

	int nframe_;

	Platform::WeakReference xamlRoot_;
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
	}
}

void NetworkManager::createFolder(const std::vector<char>& data)
{
	std::string folderName = &data[1];
	_mkdir(g_pathForFile(folderName.c_str()));
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


ApplicationManager::ApplicationManager(bool useXaml, CoreWindow^ Window, Windows::UI::Xaml::Controls::SwapChainPanel ^swapChainPanel, int width, int height, bool player, const wchar_t* resourcePath, const wchar_t* docsPath, const wchar_t* tempPath)
{
	if (swapChainPanel!=nullptr)
		xamlRoot_ = Platform::WeakReference(swapChainPanel);

	InitD3D(useXaml, Window, swapChainPanel, width, height);
	InitXAudio2();

//	if (useXaml)
//		contentScaleFactor = 1;
//	else {
		DisplayInformation ^dinfo = DisplayInformation::GetForCurrentView();
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
		contentScaleFactor = dinfo->RawPixelsPerViewPixel; // Windows phone
#else
		contentScaleFactor = ((int)dinfo->ResolutionScale)*0.01f*dinfo->LogicalDpi/96.0f;   // Windows 8 PC
#endif
//	}

	width_ = width;
	height_ = height;
	player_ = player;
	resourcePath_ = resourcePath;
	docsPath_ = docsPath;
	tempPath_ = tempPath;

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
	else
		application_->setPrintFunc(printFunc);

	application_->enableExceptions();
	application_->initialize();
	application_->setResolution(width_* contentScaleFactor, height_* contentScaleFactor);

	Binder::disableTypeChecking();

#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	hardwareOrientation_ = ePortrait;
	deviceOrientation_ = ePortrait;
#else
	if (width_>height_){
		hardwareOrientation_ = eLandscapeLeft;
		deviceOrientation_ = eLandscapeLeft;
    }
	else {
		hardwareOrientation_ = ePortrait;
		deviceOrientation_ = ePortrait;
	}
#endif

	running_ = false;

	nframe_ = 0;

	if (player_ == false)
	{
		const wchar_t *installedLocation = resourcePath_;
		char fileStem[MAX_PATH];
		wcstombs(fileStem, installedLocation, MAX_PATH);
		strcat(fileStem, "\\assets\\");
		setResourceDirectory(fileStem);

		//XXX: Redundant gpath_setDrivePath(0, fileStem);

		const wchar_t *docs = docsPath_;
		char docsPath[MAX_PATH];
		wcstombs(docsPath, docs, MAX_PATH);
		strcat(docsPath, "\\");
		setDocumentsDirectory(docsPath);

		const wchar_t *temp = tempPath_;
		char tempPath[MAX_PATH];
		wcstombs(tempPath, temp, MAX_PATH);
		strcat(tempPath, "\\");
		setTemporaryDirectory(tempPath);

		//XXX: Redundant gpath_setDrivePath(1, docsPath);

		loadProperties();

		// Gideros has became open source and free, because this, there's no more splash art
		loadLuaFiles();

	}

	next_game_tick = GetTickCount64();
}

ApplicationManager::~ApplicationManager()
{
	CleanD3D();
	CleanXAudio2();

	gaudio_Cleanup();

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

Windows::UI::Xaml::Controls::SwapChainPanel^ ApplicationManager::getRoot(){
	return xamlRoot_.Resolve<Windows::UI::Xaml::Controls::SwapChainPanel>();
}

void ApplicationManager::getStdCoords(float xp, float yp, float &x, float &y)
{
	xp = xp*contentScaleFactor;
	yp = yp*contentScaleFactor;
#if WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP

	DisplayInformation ^dinfo = DisplayInformation::GetForCurrentView();
	DisplayOrientations orientation = dinfo->CurrentOrientation;

	if (orientation == DisplayOrientations::Portrait){
		x = xp;
		y = yp;
	}
	else if (orientation == DisplayOrientations::Landscape){
		x = width_*contentScaleFactor - yp;
		y = xp;
	}
	else if (orientation == DisplayOrientations::LandscapeFlipped){
		x = yp;
		y = height_*contentScaleFactor - xp;
	}
	else {
		x = width_*contentScaleFactor - xp;
		y = height_*contentScaleFactor - yp;
	}
#else
	x = xp;
	y = yp;
#endif
}

void ApplicationManager::drawFirstFrame()
{

	application_->clearBuffers();
	application_->renderScene(1);
	drawIPs();
}

void ApplicationManager::drawFrame(bool useXaml)
{
	CoreWindow^ Window; 
	if (!useXaml) Window = CoreWindow::GetForCurrentThread();

	int FPS = g_getFps();
	if (FPS > 0) {
		if (!useXaml) Window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

		GStatus status;
		application_->enterFrame(&status);
		if (status.error())
			luaError(status.errorString());

		gaudio_AdvanceStreamBuffers();

		nframe_++;

		if (networkManager_)
			networkManager_->tick();

		if (application_->isErrorSet())
			luaError(application_->getError());

		ShaderEngine::Engine->setFramebuffer(NULL);
		application_->clearBuffers();

		application_->renderScene(1);
		drawIPs();

		if (FPS==60)	
			g_swapchain->Present(1, 0);
		else if (FPS==30)
			g_swapchain->Present(2, 0);
	}
	else {

		int absFPS = abs(FPS);

		const int MAX_FRAMESKIP = 10;
		int SKIP_TICKS = 1000 / absFPS;

		int loops = 0;
		while (GetTickCount64() > next_game_tick && loops < MAX_FRAMESKIP) {
			if (!useXaml) Window->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

			GStatus status;
			application_->enterFrame(&status);
			if (status.error())
				luaError(status.errorString());

			next_game_tick += SKIP_TICKS;
			loops++;
		}

		gaudio_AdvanceStreamBuffers();

		nframe_++;

		if (networkManager_)
			networkManager_->tick();

		//	application_->clearBuffers();  (this would duplicate ClearRenderTargetView above)

		if (application_->isErrorSet())
			luaError(application_->getError());

		ShaderEngine::Engine->setFramebuffer(NULL);
		application_->clearBuffers();
		application_->renderScene(1);
		drawIPs();

		g_swapchain->Present(1, 0);
	}
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

std::string getProjectDir(const wchar_t* base,const char *projectName)
{
	std::wstring ws(base);
	std::string dir = std::string(ws.begin(), ws.end());

	if (dir[dir.size() - 1] != '/')
		dir += "/";

	dir += "gideros";

	_mkdir(dir.c_str());

	dir += "/";

	dir += projectName;

	_mkdir(dir.c_str());

	dir += "/";
	return dir;
}

void ApplicationManager::setProjectName(const char *projectName)
{
	glog_v("setProjectName: %s", projectName);
	std::string dir = getProjectDir(docsPath_, projectName);
	std::string tdir = getProjectDir(tempPath_, projectName);

	std::string md5filename_ = dir + "md5.txt";

	std::string documents = dir + "documents";
	std::string temporary = tdir + "temporary";
	std::string resource = dir + "resource";

	glog_v("documents: %s", documents.c_str());
	glog_v("temporary: %s", temporary.c_str());
	glog_v("resource: %s", resource.c_str());

	_mkdir(documents.c_str());
	_mkdir(temporary.c_str());
	_mkdir(resource.c_str());

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

    // First arg to setResolution should be the smaller dimension
	if (width_ < height_)
		application_->setResolution(width_ * contentScaleFactor, height_ * contentScaleFactor);
	else
		application_->setResolution(height_ * contentScaleFactor, width_ * contentScaleFactor);

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

	next_game_tick = GetTickCount64();
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

	// the first arg to setResolution should be the smaller dimension
	if (width_ < height_)
		application_->setResolution(width_ * contentScaleFactor, height_ * contentScaleFactor);
	else
		application_->setResolution(height_ * contentScaleFactor, width_ * contentScaleFactor);

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

	next_game_tick = GetTickCount64();
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

	dxgiDevice->Trim();
}

void ApplicationManager::resume()
{
	gapplication_enqueueEvent(GAPPLICATION_RESUME_EVENT, NULL, 0);
	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());

	next_game_tick = GetTickCount64();
}

void ApplicationManager::exitRenderLoop()
{
	gapplication_enqueueEvent(GAPPLICATION_EXIT_EVENT, NULL, 0);

	GStatus status;
	application_->tick(&status);
	if (status.error())
		luaError(status.errorString());
}

void ApplicationManager::resize(int width, int height)
{

	//if (ShaderEngine::Engine) ShaderEngine::Engine->resizeFramebuffer(width, height);

	width_ = width;
	height_ = height;

	if (width < height){
		hardwareOrientation_ = ePortrait;
		deviceOrientation_ = ePortrait;
		application_->setResolution(width*contentScaleFactor, height*contentScaleFactor);
	}
	else {
		hardwareOrientation_ = eLandscapeLeft;
		deviceOrientation_ = eLandscapeLeft;
		application_->setResolution(height*contentScaleFactor, width*contentScaleFactor);
	}
	application_->setHardwareOrientation(hardwareOrientation_);
	application_->getApplication()->setDeviceOrientation(deviceOrientation_);
}

static ApplicationManager *s_manager = NULL;
static int lastMouseButton_ = 0;

extern "C" {

	void gdr_initialize(bool useXaml, CoreWindow^ Window, Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel, int width, int height, bool player, const wchar_t* resourcePath, const wchar_t* docsPath, const wchar_t* tempPath)
	{
		s_manager = new ApplicationManager(useXaml, Window, swapChainPanel, width, height, player, resourcePath, docsPath, tempPath);
	}

	Windows::UI::Xaml::Controls::SwapChainPanel^ gdr_getRootView(){
		return s_manager->getRoot();
	}

	void gdr_drawFrame(bool useXaml)
	{
		s_manager->drawFrame(useXaml);
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

	void gdr_keyDown(int keyCode)
	{
		ginputp_keyDown(keyCode);
	}

	void gdr_keyUp(int keyCode)
	{
		ginputp_keyUp(keyCode);
	}

	void gdr_keyChar(const char *keyChar)
	{
		ginputp_keyChar(keyChar);
	}

	void gdr_mouseDown(int x, int y, int button){
		lastMouseButton_ = button;
		float xn, yn;
		s_manager->getStdCoords(x, y, xn, yn);
		ginputp_mouseDown(xn, yn, button);
	}

	void gdr_mouseMove(int x, int y){
		float xn, yn;
		s_manager->getStdCoords(x, y, xn, yn);
		ginputp_mouseMove(xn, yn, lastMouseButton_);
	}

	void gdr_mouseHover(int x, int y){
		float xn, yn;
		s_manager->getStdCoords(x, y, xn, yn);
		ginputp_mouseHover(xn, yn, 0);
	}

	void gdr_mouseUp(int x, int y, int button){
		if (lastMouseButton_ == button)
			lastMouseButton_ = 0;
		float xn, yn;
		s_manager->getStdCoords(x, y, xn, yn);
		ginputp_mouseUp(xn, yn, button);
	}

	void gdr_mouseWheel(int x, int y, int delta){
		float xn, yn;
		s_manager->getStdCoords(x, y, xn, yn);
		ginputp_mouseWheel(xn, yn, 0, delta);
	}

	void gdr_touchBegin(int x, int y, int id){
		float xn, yn;
		s_manager->getStdCoords(x, y, xn, yn);
		ginputp_touchBegin(xn, yn, id);
	}

	void gdr_touchMove(int x, int y, int id){
		float xn, yn;
		s_manager->getStdCoords(x, y, xn, yn);
		ginputp_touchMove(xn, yn, id);
	}

	void gdr_touchEnd(int x, int y, int id){
		float xn, yn;
		s_manager->getStdCoords(x, y, xn, yn);
		ginputp_touchEnd(xn, yn, id);
	}

	void gdr_touchCancel(int x, int y, int id){
		float xn, yn;
		s_manager->getStdCoords(x, y, xn, yn);
		ginputp_touchCancel(xn, yn, id);
	}

	void gdr_resize(int width, int height)
	{
		s_manager->resize(width, height);
	}

}


#ifdef WINSTORE
extern "C"{
	char *getenv(const char *string){ return NULL; }
	int system(const char *string){ return 0; }
}
#endif

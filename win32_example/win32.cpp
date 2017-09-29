#include <iostream>
#include <windows.h>

#include "gl/glew.h"
#include "gl/glext.h"
#include "wglext.h"

#include <pluginmanager.h>
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
#include <libnetwork.h>
#include "ginput-win32.h"
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
#include "gapplication-win32.h"
#include "gaudio.h"
#include "ghttp.h"
#include "orientation.h"
#include <map>
#include <screen.h>

extern "C" {
  void g_setFps(int);
  int g_getFps();
  void setWin32Stuff(HINSTANCE hInst, HWND hwnd);
}

#define ID_TIMER   1

HWND hwndcopy;

char commandLine[256];
// int dxChrome,dyChrome;
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;
PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT;
LuaApplication *application_;

static int g_windowWidth;    // width if window was in portrait mode [used only to communicate between loadProperties and WM_CREATE]
static int g_windowHeight;   // height if window was in portrait mode > windowWidth
static bool g_portrait, drawok;
static bool use_timer=false;
static int vsyncVal=0;
static HDC hDC;

// ######################################################################

static void luaError(const char *error)
{
  glog_e("%s",error);
  exit(1);
}

// ######################################################################

static void loadPlugins()
{
  static char fullname[MAX_PATH];
  WIN32_FIND_DATA fd; 
  HANDLE hFind = FindFirstFile("plugins\\*.dll", &fd); 

  if(hFind != INVALID_HANDLE_VALUE) { 
    do { 
      // read all (real) files in current folder
      // , delete '!' read other 2 default folder . and ..
      if (! (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

	strcpy(fullname,"plugins\\");
	strcat(fullname,fd.cFileName);
	printf("found DLL: %s\n",fullname);

	HMODULE hModule = LoadLibrary(fullname);
	void* plugin = (void*)GetProcAddress(hModule,"g_pluginMain");
	if (plugin)
	  PluginManager::instance().registerPlugin((void*(*)(lua_State*, int))plugin);
      }
    } while(FindNextFile(hFind, &fd)); 
    FindClose(hFind); 
  } 
}

// ######################################################################

static void printFunc(const char *str, int len, void *data)
{
  printf("%s",str);
}

// ######################################################################

std::string getDeviceName()
{

  static char buf[MAX_COMPUTERNAME_LENGTH + 1];
  DWORD dwCompNameLen = MAX_COMPUTERNAME_LENGTH;
  std::string name;

  if (GetComputerName(buf, &dwCompNameLen) != 0) {
    name=buf;
  }

  return name;
}

// ######################################################################

struct ProjectProperties
{
  ProjectProperties()
  {
    clear();
  }

  void clear()
  {
    // graphics options
    scaleMode = 0;
    logicalWidth = 320;
    logicalHeight = 480;
    imageScales.clear();
    orientation = 0;
    fps = 60;
		
    // iOS options
    retinaDisplay = 0;
    autorotation = 0;

    // input options
    mouseToTouch = true;
    touchToMouse = true;
    mouseTouchOrder = 0;

    // export options
    architecture = 0;
    assetsOnly = false;
    iosDevice = 0;
    packageName = "com.yourdomain.yourapp";
    encryptCode = false;
    encryptAssets = false;
  }

  // graphics options
  int scaleMode;
  int logicalWidth;
  int logicalHeight;
  std::vector<std::pair<std::string, float> > imageScales;
  int orientation;
  int fps;
  
  // iOS options
  int retinaDisplay;
  int autorotation;

  // input options
  int mouseToTouch;
  int touchToMouse;
  int mouseTouchOrder;
  
  // export options
  int architecture;
  bool assetsOnly;
  int iosDevice;
  std::string packageName;
  bool encryptCode;
  bool encryptAssets;

  int windowWidth;
  int windowHeight;
};

// ### SCREENS
class W32Screen;
LRESULT CALLBACK W32Proc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
static std::map<HWND,W32Screen *> screenMap;

class W32ScreenManager : public ScreenManager {
public:
	HGLRC master;
	HDC defaultDC;
	HINSTANCE hInstance;
	W32ScreenManager(HDC dc,HGLRC gl,HINSTANCE hInstance);
	virtual Screen *openScreen(Application *application,int id);
	virtual void screenDestroyed();
};

class W32Screen : public Screen {
	virtual void tick();
	HDC dc;
	HWND wnd;
	W32FullScreen fs;
protected:
	virtual void setVisible(bool);
public:
	virtual void setSize(int w,int h);
	virtual void getSize(int &w,int &h);
	virtual void setPosition(int w,int h);
	virtual void getPosition(int &w,int &h);
	virtual void setState(int state);
	virtual int getState();
	virtual void getMaxSize(int &w,int &h);
	virtual int getId();
	virtual void closed();
	W32Screen(Application *application,HINSTANCE hInstance);
	~W32Screen();
};

void W32Screen::tick()
{
	if (!wnd) return;
	if (IsWindowVisible(wnd))
	{
		Matrix4 m;
		W32ScreenManager *sm=((W32ScreenManager *)(ScreenManager::manager));
		wglMakeCurrent(dc,sm->master);
		glBindFramebuffer(GL_FRAMEBUFFER, 0/*defaultFramebufferObject()*/);
		draw(m);
	    SwapBuffers(dc);
	}
}

void W32Screen::setSize(int w,int h)
{
	if (!wnd) return;
    RECT rect;
    rect.top=0;
    rect.left=0;
    rect.right=w;
    rect.bottom=h;

    AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
    SetWindowPos(wnd,HWND_TOP,0,0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE);
}

void W32Screen::getSize(int &w,int &h)
{
	if (!wnd) return;
 	RECT rect;
	GetClientRect(wnd,&rect);
	w=rect.right-rect.left;
	h=rect.bottom-rect.top;
}

void W32Screen::setState(int state)
{
	if (!wnd) return;
	W32SetFullScreen((state&FULLSCREEN),wnd,&fs);
	if (state&MINIMIZED) ShowWindow(wnd,SW_SHOWMINIMIZED);
	if (state&MAXIMIZED) ShowWindow(wnd,SW_SHOWMAXIMIZED);
}

int W32Screen::getState()
{
	if (!wnd) return 0;
	int s=NORMAL;
	if (fs.isFullScreen) s=FULLSCREEN;
	else
	{
		if (IsIconic(wnd)) s=MINIMIZED;
		if (IsZoomed(wnd)) s=MAXIMIZED;
	}
	if (!IsWindowVisible(wnd)) s|=HIDDEN;
	if (wnd==0) s|=CLOSED;
	return s;
}

void W32Screen::getMaxSize(int &w,int &h)
{
    MONITORINFO monitor_info;
    monitor_info.cbSize = sizeof(monitor_info);
    GetMonitorInfo(MonitorFromWindow(hwndcopy, MONITOR_DEFAULTTONEAREST),
                    &monitor_info);
	w=monitor_info.rcMonitor.right-monitor_info.rcMonitor.left;
	h=monitor_info.rcMonitor.bottom-monitor_info.rcMonitor.top;
}

void W32Screen::setPosition(int w,int h)
{
	if (!wnd) return;
    RECT rect;
    rect.top=w;
    rect.left=h;
    rect.right=0;
    rect.bottom=0;

    AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
    SetWindowPos(wnd,HWND_TOP,0,0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOSIZE);
}

void W32Screen::getPosition(int &w,int &h)
{
	if (!wnd) return;
	RECT rect;
	GetClientRect(wnd,&rect);
	w=rect.left;
	h=rect.top;
}

int W32Screen::getId()
{
	return 0;
}

void W32Screen::setVisible(bool visible)
{
	if (!wnd) return;
	ShowWindow(wnd,visible?SW_SHOW:SW_HIDE);
}

static ATOM W32Class=0;

W32Screen::W32Screen(Application *application,HINSTANCE hInstance) : Screen(application)
{
	if (!W32Class)
	{
	  WNDCLASSEX  wndclass;

	  wndclass.cbSize        = sizeof (wndclass) ;
	  wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
	  wndclass.lpfnWndProc   = W32Proc ;
	  wndclass.cbClsExtra    = 0 ;
	  wndclass.cbWndExtra    = 0 ;
	  wndclass.hInstance     = hInstance ;
	  wndclass.hIcon         = NULL;
	  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
	  wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
	  wndclass.lpszMenuName  = MAKEINTRESOURCE(100);
	  wndclass.lpszClassName = "" ;
	  wndclass.hIconSm       = NULL ;

	  W32Class=RegisterClassEx (&wndclass) ;
	}

	  wnd = CreateWindow ((LPCTSTR)W32Class,         // window class name
			       "",     // window caption
			       WS_OVERLAPPEDWINDOW,     // window style
			       0,           // initial x position
			       0,           // initial y position
			       100,           // initial x size
			       100,           // initial y size
			       NULL,                    // parent window handle
			       NULL,                    // window menu handle
			       hInstance,               // program instance handle
			       NULL) ;		             // creation parameters
	  dc=GetDC(wnd);
}

W32Screen::~W32Screen()
{
}

void W32Screen::closed()
{
	wnd=0;
}

LRESULT CALLBACK W32Proc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
  PAINTSTRUCT ps ;
  if (iMsg==WM_PAINT){

    BeginPaint (hwnd, &ps) ;
    EndPaint (hwnd, &ps) ;
    return 0 ;
  }
  else if (iMsg==WM_CLOSE){
    DestroyWindow(hwnd);
    screenMap[hwnd]->closed();
    return 0;
  }
  else if (iMsg==WM_DESTROY){
    return 0 ;
  }

  return DefWindowProc(hwnd, iMsg, wParam, lParam) ;
}


W32ScreenManager::W32ScreenManager(HDC dc,HGLRC gl,HINSTANCE hInstance)
{
	defaultDC=dc;
	master=gl;
	this->hInstance=hInstance;
}

void W32ScreenManager::screenDestroyed()
{
	wglMakeCurrent(defaultDC,master);
}


Screen *W32ScreenManager::openScreen(Application *application,int id)
{
	return new W32Screen(application,hInstance);
}

// ######################################################################

void EnableOpenGL(HWND hWnd, HDC *hDC, HGLRC *hRC)
{
  PIXELFORMATDESCRIPTOR pfd;
  int format,i;
	
  // get the device context (DC)
  *hDC = GetDC( hWnd );
	
  // set the pixel format for the DC
  ZeroMemory( &pfd, sizeof( pfd ) );
  pfd.nSize = sizeof( pfd );
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 16;
  pfd.cStencilBits = 8;
  pfd.iLayerType = PFD_MAIN_PLANE;

  format = ChoosePixelFormat( *hDC, &pfd );

  SetPixelFormat( *hDC, format, &pfd );
	
  // create and enable the render context (RC)
  *hRC = wglCreateContext( *hDC );
  wglMakeCurrent( *hDC, *hRC );

  if (! use_timer) {
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) wglGetProcAddress("wglGetExtensionsStringEXT");
    printf("wgl extensions=%s\n",_wglGetExtensionsStringEXT());
    
    if (strstr(_wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") == NULL)
    {
      printf("Extension not found WGL_EXT_swap_control\n");
    }

    wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT"); 
    
    if (wglSwapIntervalEXT == NULL){
      printf("No wglSwapIntervalEXT, reverting to timer events\n");
      use_timer=true;
      return;
    }
    //    wglSwapIntervalEXT(vsyncVal);

    wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC) wglGetProcAddress("wglGetSwapIntervalEXT");

    if (wglGetSwapIntervalEXT == NULL){
      printf("No wglGetSwapIntervalEXT\n");
    }
    //    printf("wglGetSwapIntervalEXT=%d\n",wglGetSwapIntervalEXT());

  }

}

// ######################################################################

void DisableOpenGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
  wglMakeCurrent( NULL, NULL );
  wglDeleteContext( hRC );
  ReleaseDC( hWnd, hDC );
}

// ######################################################################

void loadLuaFiles()
{
  std::vector<std::string> luafiles;

  G_FILE* fis = g_fopen("luafiles.txt", "rt");

  if (fis){
    char line[1024];
    while (true){
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
  for (size_t i = 0; i < luafiles.size(); ++i){
    application_->loadFile(luafiles[i].c_str(), &status);
    if (status.error())
      break;
  }

  if (!status.error()){
    gapplication_enqueueEvent(GAPPLICATION_START_EVENT, NULL, 0);
    application_->tick(&status);
  }

  if (status.error())
    luaError(status.errorString());
}

// ######################################################################

bool loadProperties()
{
  ProjectProperties properties;
  G_FILE* fis = g_fopen("properties.bin", "rb");
  if (!fis) return false;

  g_fseek(fis, 0, SEEK_END);
  int len = g_ftell(fis);
  g_fseek(fis, 0, SEEK_SET);
  
  std::vector<char> buf(len);
  g_fread(&buf[0], 1, len, fis);
  g_fclose(fis);
  
  ByteBuffer buffer(&buf[0], buf.size());
  
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

  //  properties.scaleMode=3; (letterbox) for testing

  printf("properties components\n");
  printf("logicalWidth, logicalHeight, orientation, scaleMode=%d %d %d %d\n",
	 properties.logicalWidth, properties.logicalHeight, 
	 properties.orientation, properties.scaleMode);

  buffer >> properties.windowWidth;
  buffer >> properties.windowHeight;

  printf("windowWidth, windowHeight=%d %d\n",properties.windowWidth, properties.windowHeight);

  if (properties.windowWidth==0 || properties.windowHeight==0){
    g_windowWidth=properties.logicalWidth;
    g_windowHeight=properties.logicalHeight;
  }    
  else {
    g_windowWidth=properties.windowWidth;
    g_windowHeight=properties.windowHeight;
  }

  float contentScaleFactor = 1;
  Orientation hardwareOrientation;
  Orientation deviceOrientation;

  if (properties.orientation==0 || properties.orientation==2){
    hardwareOrientation = ePortrait;
    deviceOrientation = ePortrait;
    g_portrait=true;
  }
  else {
    hardwareOrientation = eLandscapeLeft;
    deviceOrientation = eLandscapeLeft;
    g_portrait=false;
  }

  application_->setResolution(g_windowWidth * contentScaleFactor, 
			      g_windowHeight * contentScaleFactor);

  application_->setHardwareOrientation(hardwareOrientation);                     // previously eFixed
  application_->getApplication()->setDeviceOrientation(deviceOrientation);     // previously eFixed
  application_->setOrientation((Orientation)properties.orientation);
  application_->setLogicalDimensions(properties.logicalWidth, properties.logicalHeight);
  application_->setLogicalScaleMode((LogicalScaleMode)properties.scaleMode);
  application_->setImageScales(properties.imageScales);
  
  g_setFps(properties.fps);

  ginput_setMouseToTouchEnabled(properties.mouseToTouch);
  ginput_setTouchToMouseEnabled(properties.touchToMouse);
  ginput_setMouseTouchOrder(properties.mouseTouchOrder);
 return true;
}

// ######################################################################

static HINSTANCE hInst;
static bool isPlayer_;
static Server *server_=NULL;

void netTick() {
    ScreenManager::manager->tick();
}
LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

  static HGLRC hRC;
  static RECT clientRect,winRect;

  PAINTSTRUCT ps ;

  if (iMsg==WM_CREATE){

    EnableOpenGL(hwnd, &hDC, &hRC);

    if (glewInit()){
      printf("glewInit failed to initialise!\n");
      exit(1);
    }

    hwndcopy=hwnd;                  // for platform-win32

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

    char resourcePath[MAX_PATH];
    strcpy(resourcePath,"assets\\");
    gpath_setDrivePath(0,resourcePath);
    
    char docsPath[MAX_PATH];
    strcpy(docsPath,getenv("APPDATA"));
    strcat(docsPath, "\\giderosgame\\");
    CreateDirectory(docsPath,NULL);        // create dir if it does not exist
    gpath_setDrivePath(1,docsPath);

    gvfs_init();
    //    gvfs_setCodeKey(codeKey_ + 32);
    //    gvfs_setAssetsKey(assetsKey_ + 32);
	
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

    loadPlugins();

    application_ = new LuaApplication;

    application_->enableExceptions();
    application_->initialize();
    application_->setPrintFunc(printFunc);
    
    if (!loadProperties())
    {
    	isPlayer_=true;
    	g_portrait=false;
    	g_windowWidth=320;
    	g_windowHeight=480;
    }

    //    GetClientRect(hwnd,&clientRect);
    //    GetWindowRect(hwnd,&winRect);

    //    dxChrome=winRect.right-winRect.left-(clientRect.right-clientRect.left);
    //    dyChrome=winRect.bottom-winRect.top-(clientRect.bottom-clientRect.top);

    if (g_portrait){
      RECT rect;
      rect.top=0;
      rect.left=0;
      rect.right=g_windowWidth;
      rect.bottom=g_windowHeight;

      printf("WM_CREATE: portrait input rect = %d %d\n",rect.right,rect.bottom);
      AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
      printf("adjusted rect = %d %d %d %d\n",rect.left,rect.top,rect.right,rect.bottom);

      //      SetWindowPos(hwnd,HWND_TOP,0,0,g_windowWidth+dxChrome+13,g_windowHeight+dyChrome+13,SWP_NOMOVE);
      SetWindowPos(hwnd,HWND_TOP,0,0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE);
    }
    else {
      RECT rect;
      rect.left=0;
      rect.top=0;
      rect.right=g_windowHeight;
      rect.bottom=g_windowWidth;

      printf("WM_CREATE landscape input rect = %d %d\n",rect.right,rect.bottom);
      AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
      printf("adjusted rect = %d %d %d %d\n",rect.left,rect.top,rect.right,rect.bottom);

//      SetWindowPos(hwnd,HWND_TOP,0,0,g_windowHeight+dxChrome+14,g_windowWidth+dyChrome+14,SWP_NOMOVE);
      SetWindowPos(hwnd,HWND_TOP,0,0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE);
    }
	ScreenManager::manager=new W32ScreenManager(hDC,hRC,hInst);

	application_->setPlayerMode(isPlayer_);
    if (isPlayer_) {
        //application_->setPrintFunc(printToServer);
		server_ = new Server(0, ::getDeviceName().c_str()); //Default port
    }
    else
    	loadLuaFiles();
    printf("Loaded Lua files\n");

    return 0;
  }
  else if (iMsg==WM_SIZE){

    int width=LOWORD(lParam);
    int height=HIWORD(lParam);

    //    printf("WM_SIZE: %d x %d\n",width,height);

    int windowWidth, windowHeight;

    if (application_->orientation()==ePortrait || application_->orientation()==ePortraitUpsideDown){              // previously width < height
      windowWidth=width;
      windowHeight=height;
    }
    else {
      windowWidth=height;
      windowHeight=width;
    }

    float contentScaleFactor = 1;
    application_->setResolution(windowWidth  * contentScaleFactor, 
				windowHeight * contentScaleFactor);
    
    return 0;
  }
  // allows large windows bigger than screen
  else if (iMsg==WM_GETMINMAXINFO) {
    DefWindowProc(hwnd, iMsg, wParam, lParam);
    MINMAXINFO* pmmi = (MINMAXINFO*)lParam;
    pmmi->ptMaxTrackSize.x = 2000;
    pmmi->ptMaxTrackSize.y = 2000;
    return 0;
  }
  else if (iMsg==WM_LBUTTONDOWN){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
    ginputp_mouseDown(LOWORD(lParam), HIWORD(lParam), 0,m);
    return 0;
  }
  else if (iMsg==WM_LBUTTONUP){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
    ginputp_mouseUp(LOWORD(lParam), HIWORD(lParam), 0,m);
    return 0;
  }
  else if (iMsg==WM_MOUSEMOVE && wParam & MK_LBUTTON !=0){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
    ginputp_mouseMove(LOWORD(lParam), HIWORD(lParam),m);
    return 0;
  }
  else if (iMsg==WM_MOUSEWHEEL){
	  int m=0;
	  if (wParam&MK_CONTROL) m|=GINPUT_CTRL_MODIFIER;
	  if (wParam&MK_SHIFT) m|=GINPUT_SHIFT_MODIFIER;
    ginputp_mouseWheel(LOWORD(lParam), HIWORD(lParam), 0,HIWORD(wParam),m);
    return 0;
  }
  else if (iMsg==WM_KEYDOWN){
    ginputp_keyDown(wParam);
    return 0;
  }
  else if (iMsg==WM_KEYUP){
    ginputp_keyUp(wParam);
    return 0;
  }
  else if (iMsg==WM_PAINT){
	netTick();
    BeginPaint (hwnd, &ps) ;

    //    glClear(GL_COLOR_BUFFER_BIT);
    application_->clearBuffers();
    application_->renderScene(1);

    SwapBuffers(hDC);

    EndPaint (hwnd, &ps) ;
    return 0 ;
  }
  else if (iMsg==WM_TIMER){
    //    gaudio_AdvanceStreamBuffers();
    netTick();
    GStatus status;
    application_->enterFrame(&status);

    if (status.error())
      luaError(status.errorString());

    //    glClear(GL_COLOR_BUFFER_BIT);
    application_->clearBuffers();

    application_->renderScene();

    SwapBuffers(hDC);

    return 0;
  }
  else if (iMsg==WM_CLOSE){
    printf("WM_CLOSE Called\n");

    if (use_timer)
      KillTimer(hwnd, ID_TIMER);
    else {
      PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT"); 
    
      if (wglSwapIntervalEXT == NULL){
	printf("Error, no wglSwapIntervalEXT\n");
	exit(1);
      }
      wglSwapIntervalEXT(0);
    }
    
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

    DisableOpenGL(hwnd, hDC, hRC);

    drawok=false;

    DestroyWindow(hwnd);
    return 0;
  }
  else if (iMsg==WM_DESTROY){
    printf("WM_DESTROY Called\n");
    PostQuitMessage (0) ;

    return 0 ;
  }

  return DefWindowProc(hwnd, iMsg, wParam, lParam) ;
}

// ######################################################################

void render()
{
  GStatus status;
  application_->enterFrame(&status);
  
  if (status.error())
    luaError(status.errorString());

  //    glClear(GL_COLOR_BUFFER_BIT);
  application_->clearBuffers();
  
  application_->renderScene();
  
  SwapBuffers(hDC);
}

// ######################################################################

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
  static char szAppName[] = "giderosGame" ;
  HWND        hwnd ;
  MSG         msg ;
  WNDCLASSEX  wndclass ;
  int ret;

  printf("szCmdLine=%s\n",szCmdLine);

  if (strcmp(szCmdLine,"timer")==0){
    printf("Using timer as requested\n");
    use_timer=true;
  }
  else {
    printf("Will use VSYNC if available\n");
    use_timer=false;
  }

  wndclass.cbSize        = sizeof (wndclass) ;
  wndclass.style         = CS_HREDRAW | CS_VREDRAW ;
  wndclass.lpfnWndProc   = WndProc ;
  wndclass.cbClsExtra    = 0 ;
  wndclass.cbWndExtra    = 0 ;
  wndclass.hInstance     = hInstance ;
  wndclass.hIcon         = NULL;
  wndclass.hCursor       = LoadCursor (NULL, IDC_ARROW) ;
  wndclass.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH) ;
  wndclass.lpszMenuName  = MAKEINTRESOURCE(100);
  wndclass.lpszClassName = szAppName ;
  wndclass.hIconSm       = NULL ;
  
  RegisterClassEx (&wndclass) ;

  hInst=hInstance;
  hwnd = CreateWindow (szAppName,         // window class name
		       "Gideros Win32 (no Qt)",     // window caption
		       WS_OVERLAPPEDWINDOW,     // window style
		       0,           // initial x position
		       0,           // initial y position
		       100,           // initial x size
		       100,           // initial y size
		       NULL,                    // parent window handle
		       NULL,                    // window menu handle
		       hInstance,               // program instance handle
		       NULL) ;		             // creation parameters

  setWin32Stuff(hInstance,hwnd);

  // ----------------------------------------------------------------------
  // Create window
  // ----------------------------------------------------------------------

  ShowWindow (hwnd, iCmdShow) ;
  //  UpdateWindow (hwnd) ;

  int fps;
  if (use_timer){

    fps=-1;

    while (GetMessage (&msg, NULL, 0, 0)) {

      if (g_getFps() != fps){
	fps=g_getFps();
	if (fps==30)
	  SetTimer(hwnd, ID_TIMER, 30, NULL);
	else if (fps==60)
	  SetTimer(hwnd, ID_TIMER, 10, NULL);   // 10 is the minimum, actually more like 16 ms.
	else {
	  printf("Illegal FPS (timer): %d\n",fps);
	  exit(1);
	}
      }

      TranslateMessage (&msg) ;
      DispatchMessage (&msg) ;
    }
  }
  else {

    fps=-1;

    drawok=true;

    while (TRUE) {

      if (g_getFps() != fps){
	fps=g_getFps();
	if (fps==30)
	  wglSwapIntervalEXT(2);
	else if (fps==60)
	  wglSwapIntervalEXT(1);
	else {
	  printf("Illegal FPS (VSYNC): %d\n",fps);
	  exit(1);
	}
      }

      if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
	if (msg.message == WM_QUIT){
	  printf("WM_QUIT message received\n");
	  break ;
	}

	TranslateMessage (&msg) ;
	DispatchMessage (&msg) ;
      }

      if (drawok) render();
    }
  }

  printf("program ends\n");
  return msg.wParam ;
}


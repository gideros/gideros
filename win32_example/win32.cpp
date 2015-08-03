#include <iostream>
#include <windows.h>

#include "gl/glew.h"
#include "gl/glext.h"
#include "wglext.h"

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

LuaApplication *application_;
HWND hwndcopy;

int g_windowWidth;    // width if window was in portrait mode
int g_windowHeight;   // height if window was in portrait mode > windowWidth
bool g_portrait;

#define ID_TIMER   1
bool use_timer=false;
int vsyncVal=0;
static HDC hDC;

static void printFunc(const char *str, int len, void *data)
{
  printf("%s",str);
}

std::string getDeviceName()
{
  return "foo";
}

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

// ######################################################################

void EnableOpenGL(HWND hWnd, HDC * hDC, HGLRC * hRC)
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
  pfd.iLayerType = PFD_MAIN_PLANE;

  format = ChoosePixelFormat( *hDC, &pfd );

  SetPixelFormat( *hDC, format, &pfd );
	
  // create and enable the render context (RC)
  *hRC = wglCreateContext( *hDC );
  wglMakeCurrent( *hDC, *hRC );

  if (not use_timer) {
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC) wglGetProcAddress("wglGetExtensionsStringEXT");
    printf("wgl extensions=%s\n",_wglGetExtensionsStringEXT());
    
    if (strstr(_wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") == NULL)
    {
      printf("Extension not found WGL_EXT_swap_control\n");
      exit(1);
    }

    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT"); 
    
    if (wglSwapIntervalEXT == NULL){
      printf("Error, no wglSwapIntervalEXT\n");
      exit(1);
    }
    wglSwapIntervalEXT(vsyncVal);

    PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC) wglGetProcAddress("wglGetSwapIntervalEXT");

    if (wglGetSwapIntervalEXT == NULL){
      printf("Error, no wglGetSwapIntervalEXT\n");
      exit(1);
    }
    printf("wglGetSwapIntervalEXT=%d\n",wglGetSwapIntervalEXT());

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

  if (status.error());
    //    luaError(status.errorString());
}

// ######################################################################

void loadProperties()
{
  ProjectProperties properties;
  G_FILE* fis = g_fopen("properties.bin", "rb");

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

  printf("properties components\n");
  printf("logicalWidth, logicalHeight, orientation=%d %d %d\n",
	 properties.logicalWidth, properties.logicalHeight, properties.orientation);

  buffer >> properties.windowWidth;
  buffer >> properties.windowHeight;

  printf("windowWidth, windowHeight=%d %d\n",properties.windowWidth, properties.windowHeight);

  //    if (properties.orientation==0 || properties.orientation==2){            // portrait

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

  // the first arg to setResolution should be the smaller dimension
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
  //    application_->setResolution(windowHeight * contentScaleFactor, windowWidth * contentScaleFactor);


  application_->setHardwareOrientation(hardwareOrientation);
  application_->getApplication()->setDeviceOrientation(deviceOrientation);
  application_->setOrientation((Orientation)properties.orientation);
  application_->setLogicalDimensions(properties.logicalWidth, properties.logicalHeight);
  application_->setLogicalScaleMode((LogicalScaleMode)properties.scaleMode);
  application_->setImageScales(properties.imageScales);
  
  //  g_setFps(properties.fps);

  ginput_setMouseToTouchEnabled(properties.mouseToTouch);
  ginput_setTouchToMouseEnabled(properties.touchToMouse);
  ginput_setMouseTouchOrder(properties.mouseTouchOrder);
}

// ######################################################################

LRESULT CALLBACK WndProc (HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{

  static HGLRC hRC;
  static RECT clientRect,winRect;

  static int dxChrome,dyChrome;

  PAINTSTRUCT ps ;

  if (iMsg==WM_CREATE){

    EnableOpenGL(hwnd, &hDC, &hRC);

    if (glewInit()){
      printf("glewInit failed to initialise!\n");
      exit(1);
    }

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

    application_ = new LuaApplication;

    application_->enableExceptions();
    application_->initialize();
    application_->setPrintFunc(printFunc);
    
    loadProperties();
    loadLuaFiles();

    GetClientRect(hwnd,&clientRect);
    GetWindowRect(hwnd,&winRect);

    dxChrome=winRect.right-winRect.left-(clientRect.right-clientRect.left);
    dyChrome=winRect.bottom-winRect.top-(clientRect.bottom-clientRect.top);

    if (g_portrait)
      SetWindowPos(hwnd,HWND_TOP,0,0,g_windowWidth+dxChrome,g_windowHeight+dyChrome,SWP_NOMOVE);
    else
      SetWindowPos(hwnd,HWND_TOP,0,0,g_windowHeight+dxChrome,g_windowWidth+dyChrome,SWP_NOMOVE);

    return 0;
  }
  else if (iMsg==WM_SIZE){
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
    ginputp_mouseDown(LOWORD(lParam), HIWORD(lParam), 0);
    return 0;
  }
  else if (iMsg==WM_LBUTTONUP){
    ginputp_mouseUp(LOWORD(lParam), HIWORD(lParam), 0);
    return 0;
  }
  else if (iMsg==WM_MOUSEMOVE && wParam & MK_LBUTTON !=0){
    ginputp_mouseMove(LOWORD(lParam), HIWORD(lParam));
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

    GStatus status;
    application_->enterFrame(&status);

    if (status.error());
    //      luaError(status.errorString());

    //    glClear(GL_COLOR_BUFFER_BIT);
    application_->clearBuffers();

    application_->renderScene();

    SwapBuffers(hDC);

    return 0;
  }
  else if (iMsg==WM_CLOSE){
    printf("WM_CLOSE Called\n");

    //    gaudio_Cleanup();

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
    
    DestroyWindow(hwnd);
    return 0;
  }
  else if (iMsg==WM_DESTROY){
    PostQuitMessage (0) ;
    return 0 ;
  }

  return DefWindowProc(hwnd, iMsg, wParam, lParam) ;
}

void render()
{
  GStatus status;
  application_->enterFrame(&status);
  
  if (status.error());
  //      luaError(status.errorString());

  //    glClear(GL_COLOR_BUFFER_BIT);
  application_->clearBuffers();
  
  application_->renderScene();
  
  SwapBuffers(hDC);
}

// ######################################################################

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
  static char szAppName[] = "triangles" ;
  HWND        hwnd ;
  MSG         msg ;
  WNDCLASSEX  wndclass ;

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

  printf("OpenGL version=%s\n",glGetString(GL_VERSION));
  printf("szCmdLine=%s\n",szCmdLine);

  sscanf(szCmdLine,"%d",&vsyncVal);
  printf("vsyncVal=%d\n",vsyncVal);

  if (vsyncVal==0)
    use_timer=true;
  else
    use_timer=false;
  
  hwnd = CreateWindow (szAppName,         // window class name
		       "Gideros Win32 (no Qt)",     // window caption
		       WS_OVERLAPPED | WS_SYSMENU,     // window style
		       CW_USEDEFAULT,           // initial x position
		       CW_USEDEFAULT,           // initial y position
		       480,           // initial x size
		       320,           // initial y size
		       NULL,                    // parent window handle
		       NULL,                    // window menu handle
		       hInstance,               // program instance handle
		       NULL) ;		             // creation parameters

  hwndcopy=hwnd;

  // ----------------------------------------------------------------------
  // Create window
  // ----------------------------------------------------------------------

  ShowWindow (hwnd, iCmdShow) ;
  //  UpdateWindow (hwnd) ;

  if (use_timer){

    SetTimer(hwnd, ID_TIMER, 0, NULL);

    while (GetMessage (&msg, NULL, 0, 0)) {
      TranslateMessage (&msg) ;
      DispatchMessage (&msg) ;
    }
  }
  else {
    while (TRUE) {
      if (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE)) {
	if (msg.message == WM_QUIT)
	  break ;
	
	TranslateMessage (&msg) ;
	DispatchMessage (&msg) ;
      }

      render();
    }
  }

  return msg.wParam ;
}


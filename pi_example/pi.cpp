#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <fstream>
#include <math.h>
#include <stack>
#include <string>
#include <iostream>

#include <pluginmanager.h>
#include <binder.h>
#include <libnetwork.h>
#include "ginput-pi.h"
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
#include "gapplication-pi.h"
#include "gaudio.h"
#include "ghttp.h"
#include "orientation.h"

#include "esUtil.h"

#ifdef RPI_NO_X
#include  "bcm_host.h"
#else
#include  <X11/Xlib.h>
#include  <X11/Xatom.h>
#include  <X11/Xutil.h>
#endif

#ifndef RPI_NO_X
// X11 related local variables
static Display *x_display = NULL;
#endif

extern "C" {
  void g_setFps(int);
  int g_getFps();
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

// ######################################################################

typedef struct
{
   // Handle to a program object
   GLuint programObject;

} UserData;

// ######################################################################

static LuaApplication *application_;
static int g_windowWidth;    // width if window was in portrait mode
static int g_windowHeight;   // height if window was in portrait mode > windowWidth
static bool g_portrait, drawok;

// ######################################################################

std::string getDeviceName()
{
  return "placeholder";
}

// ######################################################################

static void luaError(const char *error)
{
  glog_e("%s",error);
  exit(1);
}

// ######################################################################

static void printFunc(const char *str, int len, void *data)
{
  printf("%s",str);
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

  application_->setHardwareOrientation(hardwareOrientation);
  application_->getApplication()->setDeviceOrientation(deviceOrientation);
  application_->setOrientation((Orientation)properties.orientation);
  application_->setLogicalDimensions(properties.logicalWidth, properties.logicalHeight);
  application_->setLogicalScaleMode((LogicalScaleMode)properties.scaleMode);
  application_->setImageScales(properties.imageScales);
  
  g_setFps(properties.fps);

  ginput_setMouseToTouchEnabled(properties.mouseToTouch);
  ginput_setTouchToMouseEnabled(properties.touchToMouse);
  ginput_setMouseTouchOrder(properties.mouseTouchOrder);
}

// ######################################################################

int main(int argc, char *argv[])
{
   ESContext esContext;
   UserData  userData;

   esInitContext ( &esContext );
   esContext.userData = &userData;

   esCreateWindow ( &esContext, "Hello Triangle", 960, 640, ES_WINDOW_RGB );

//   if ( !Init ( &esContext ) )
//      return 0;

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

    gpath_setDrivePath(0,"assets/");
    
    gpath_setDrivePath(1,"docs/");

    printf("pi call gvfs_init\n");
    gvfs_init();
	
    // event
    printf("pi call gevent_Init\n");
    gevent_Init();

    // application
    printf("pi call gapplication_init\n");
    gapplication_init();
    
    // input
    printf("pi call ginput_init\n");
    ginput_init();
    
    // geolocation
    printf("pi call ggeolocation_init\n");
    ggeolocation_init();
    
    // http
    printf("pi call ghttp_init\n");
    ghttp_Init();
    
    // ui
    printf("pi call gui_init\n");
    gui_init();
    
    // texture
    printf("pi call gtexture_init\n");
    gtexture_init();
    
    // audio
    printf("pi call gaudio_Init\n");
    gaudio_Init();

    // loadPlugins();

    application_ = new LuaApplication;
    printf("created application_\n");

    application_->enableExceptions();
    printf("enabled exceptions\n");

    application_->initialize();
    printf("initialized application_\n");

    application_->setPrintFunc(printFunc);
    printf("setPrintFunc_\n");
    
    loadProperties();
    printf("loaded properties\n");

    loadLuaFiles();

    printf("loaded lua files\n");

    struct timeval t1, t2;
    struct timezone tz;
    float deltatime;
    float totaltime = 0.0f;
    unsigned int frames = 0;

    gettimeofday ( &t1 , &tz );

    while(1)
    {

        gettimeofday(&t2, &tz);
        deltatime = (float)(t2.tv_sec - t1.tv_sec + (t2.tv_usec - t1.tv_usec) * 1e-6);
        t1 = t2;

        GStatus status;
        application_->enterFrame(&status);
  
        if (status.error())
          luaError(status.errorString());

        application_->clearBuffers();
        application_->renderScene();

//        if (esContext->updateFunc != NULL)
//           esContext->updateFunc(esContext, deltatime);
//        if (esContext->drawFunc != NULL)
//            esContext->drawFunc(esContext);

        eglSwapBuffers(esContext.eglDisplay, esContext.eglSurface);

        totaltime += deltatime;
        frames++;
        if (totaltime >  2.0f)
        {
            printf("%4d frames rendered in %1.4f seconds -> FPS=%3.4f\n", frames, totaltime, frames/totaltime);
            totaltime -= 2.0f;
            frames = 0;
        }
    }


//   esRegisterDrawFunc ( &esContext, Draw );

//   esMainLoop ( &esContext );
   return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/time.h>
#include "GL/glew.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <math.h>
#include <stack>
#include <string>
#include <iostream>
#include <dlfcn.h>
#include <dirent.h>

#include <applicationmanager.h>
#include "luaapplication.h"
#include "platform.h"
#include "refptr.h"
#include <application.h>

#include "unistd.h"
#include "limits.h"
#include "glog.h"
#include "gplugin.h"

static GLFWwindow *glfw_win;
float pixelRatio=1.0;
int lastGLWidth=0,lastGLHeight=0;

std::string PATH_Executable;
std::string PATH_Temp;
std::string PATH_Cache;
std::string PATH_AppName;

extern "C" {
  void g_setFps(int);
  int g_getFps();
}

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

static void loadPlugins() {
	std::string ppath=PATH_Executable+"/plugins";
	DIR *pdir=opendir(ppath.c_str());
	if (pdir) {
		struct dirent *de;
		while ((de=readdir(pdir))!=NULL) {
			int dl=strlen(de->d_name);
			if ((dl>3)&&(!strcmp(".so",de->d_name+dl-3))) {
				std::string pfile=ppath+"/"+de->d_name;
				printf("Plugin:%s\n",pfile.c_str());
				void *dlib=dlopen(pfile.c_str(),RTLD_LAZY);
				if (dlib) {
					void *(*func)(lua_State *,int)=(void *(*)(lua_State *,int))dlsym(dlib,"g_pluginMain");
					 int ret=0;
					 if (func)
					  ret=g_registerPlugin(func);
					 else
					  fprintf(stderr,"Entry point not found\n");
				}
			}
		}
		closedir(pdir);
	}
}

int initGL(int &width, int &height)
{
 if (glfwInit() != GL_TRUE) {
  printf("glfwInit() failed\n");
  return GL_FALSE;
 }

 
 pixelRatio=1.0;//emscripten_get_device_pixel_ratio();
 printf("Resize:%d,%d\n",width,height);
 lastGLWidth=width;
 lastGLHeight=height;
 width*=pixelRatio;
 height*=pixelRatio;
 printf("CanvasSize: %d,%d (%f)\n",width,height,pixelRatio);
                      
 //8, 8, 8, 8, 16, 8, GLFW_WINDOW
 //glfwWindowHint(,);
 glfw_win = glfwCreateWindow(width, height, "", NULL, NULL);
 glfwMakeContextCurrent(glfw_win);

	if (glewInit()!=GLEW_OK) {
		printf("glewInit failed\n");
		return GL_FALSE;
	}
 
 return 0;
}

int defWidth, defHeight;
bool resized;
void cb_winsize(GLFWwindow *win,int w,int h) {
	defWidth=w;
	defHeight=h;
	resized=true;
}
void cb_cursorpos(GLFWwindow *win,double x,double y) {
	printf("MOUSEM:%f,%f\n",x,y);
}
void cb_cursorenter(GLFWwindow *win,int enter) {
	printf("MOUSEE:%d\n",enter);
}
void cb_mousebtn(GLFWwindow *win,int btn,int act,int mods) {
	printf("MOUSEB:%d,%d,%d\n",btn,act,mods);
}
// ######################################################################
static ApplicationManager *s_applicationManager;
int main(int argc, char *argv[])
{	
  PATH_AppName="LinuxTemplate";
  char pathName[PATH_MAX+1];
  readlink("/proc/self/exe",pathName,PATH_MAX);
  PATH_Executable=pathName;
  PATH_Executable=PATH_Executable.substr(0,PATH_Executable.rfind('/'));
  PATH_Temp="/tmp";
  PATH_Cache=PATH_Temp+"/"+PATH_AppName;


	defWidth=320;
	defHeight=480;
	initGL(defWidth,defHeight);
	resized=false;

    loadPlugins();
	
	s_applicationManager=new ApplicationManager();
	s_applicationManager->surfaceCreated();
	
	glfwSetWindowSizeCallback(glfw_win,cb_winsize);
	glfwSetCursorPosCallback(glfw_win,cb_cursorpos);
	glfwSetCursorEnterCallback(glfw_win,cb_cursorenter);
	glfwSetMouseButtonCallback(glfw_win,cb_mousebtn);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(glfw_win,GLFW_RAW_MOUSE_MOTION,GLFW_TRUE);
		
	
	glfwMakeContextCurrent(glfw_win);
    while(!glfwWindowShouldClose(glfw_win))
    {
		if (resized) {
		  s_applicationManager->surfaceChanged(defWidth,defHeight,(defWidth>defHeight)?90:0);
		}

		s_applicationManager->drawFrame();

        glfwSwapBuffers(glfw_win);
		glfwPollEvents();

    }

	glfwDestroyWindow(glfw_win);
	glfwTerminate();

//   esRegisterDrawFunc ( &esContext, Draw );

//   esMainLoop ( &esContext );
   return 0;
}

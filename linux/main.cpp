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
#include "ginput-linux.h"
#include "ghttp-linux.h"

static ApplicationManager *s_applicationManager;
LuaApplication *application_;

static const char szAppName[256] = "LinuxTemplateDir" ;
static const char szAppTitle[256] = "Linux Template App Name" ;

GLFWwindow *glfw_win;
float pixelRatio=1.0;
int lastGLWidth=0,lastGLHeight=0;

std::string PATH_Executable;
std::string PATH_Temp;
std::string PATH_Cache;
std::string PATH_AppName;
std::vector<std::string> PATH_CommandLine;

extern "C" {
  void g_setFps(int);
  int g_getFps();
}

// ######################################################################

std::string getDeviceName()
{
	char Name[256];
	memset(Name,0,256);
	gethostname(Name,256);
	return Name;
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
 glfw_win = glfwCreateWindow(width, height, szAppTitle, NULL, NULL);
 glfwMakeContextCurrent(glfw_win);

	if (glewInit()!=GLEW_OK) {
		printf("glewInit failed\n");
		return GL_FALSE;
	}
 
 return 0;
}

int defWidth, defHeight;
bool resized;
int mButtons=0,mMods=0;
double mpos_x=0,mpos_y=0;
void cb_winsize(GLFWwindow *win,int w,int h) {
	defWidth=w;
	defHeight=h;
	resized=true;
}
int getMods(GLFWwindow *win) {
	int mmap=0;
	if (glfwGetKey(win,GLFW_KEY_LEFT_SHIFT)||glfwGetKey(win,GLFW_KEY_RIGHT_SHIFT))
		mmap|=GINPUT_SHIFT_MODIFIER;
	if (glfwGetKey(win,GLFW_KEY_LEFT_ALT)||glfwGetKey(win,GLFW_KEY_RIGHT_ALT))
		mmap|=GINPUT_ALT_MODIFIER;
	if (glfwGetKey(win,GLFW_KEY_LEFT_CONTROL)||glfwGetKey(win,GLFW_KEY_RIGHT_CONTROL))
		mmap|=GINPUT_CTRL_MODIFIER;
	if (glfwGetKey(win,GLFW_KEY_LEFT_SUPER)||glfwGetKey(win,GLFW_KEY_RIGHT_SUPER))
		mmap|=GINPUT_META_MODIFIER;
	return mmap;
}

void cb_key(GLFWwindow *win,int key,int scan,int action, int mods)
{
	int mmap=0;
	if (mods&GLFW_MOD_SHIFT)
		mmap|=GINPUT_SHIFT_MODIFIER;
	if (mods&GLFW_MOD_ALT)
		mmap|=GINPUT_ALT_MODIFIER;
	if (mods&GLFW_MOD_CONTROL)
		mmap|=GINPUT_CTRL_MODIFIER;
	if (mods&GLFW_MOD_SUPER)
		mmap|=GINPUT_META_MODIFIER;
	if (action==GLFW_PRESS) {
		ginputp_keyDown(key,mmap);
		if (key==GLFW_KEY_TAB)
			ginputp_keyChar("\t");
		else if (key==GLFW_KEY_ENTER)
			ginputp_keyChar("\r");
		else if (key==GLFW_KEY_BACKSPACE)
			ginputp_keyChar("\b");
	}
	else if (action==GLFW_RELEASE)
		ginputp_keyUp(key,mmap);
	mMods=getMods(win);
	//printf("CBK:%d,%d %d %x\n",key,scan,action,mMods);
}
void cb_char(GLFWwindow *win,unsigned int uni) {
	char sc[4];
	char *obuf=sc;
	 if (uni<0x80)
	  *(obuf++)=uni;
	 else
	 {
	  if (uni<0x800)
	  {
	   *(obuf++)=0xC0|(uni>>6);
	   *(obuf++)=0x80|(uni&0x3F);
	  }
	  else
	  {
	    *(obuf++)=0xE0|(uni>>12);
	    *(obuf++)=0x80|((uni>>6)&0x3F);
	    *(obuf++)=0x80|(uni&0x3F);
	  }
	 }
	 *(obuf++)=0;
	 ginputp_keyChar(sc);
	//printf("CBC:%d [%s]\n",uni,sc);
}

void cb_cursorpos(GLFWwindow *win,double x,double y) {
	mpos_x=x;
	mpos_y=y;
	if (mButtons) 
		ginputp_mouseMove(x,y,mButtons,mMods);
	else
		ginputp_mouseHover(x,y,mButtons,mMods);
	//printf("MOUSEM:%f,%f\n",x,y);
}
void cb_cursorenter(GLFWwindow *win,int enter) {
	double x,y;
	glfwGetCursorPos(win,&x,&y);
	if (enter)
		ginputp_mouseEnter(x,y,0,mMods);
	else
		ginputp_mouseLeave(0,0,mMods);
	//printf("MOUSEE:%d\n",enter);
}
void cb_mousebtn(GLFWwindow *win,int btn,int act,int mods) {
	int bmap=0;
	if (btn==GLFW_MOUSE_BUTTON_LEFT)
		bmap=GINPUT_LEFT_BUTTON;
	else if (btn==GLFW_MOUSE_BUTTON_RIGHT)
		bmap=GINPUT_RIGHT_BUTTON;
	else if (btn==GLFW_MOUSE_BUTTON_MIDDLE)
		bmap=GINPUT_MIDDLE_BUTTON;
	int mmap=0;
	if (mods&GLFW_MOD_SHIFT)
		mmap|=GINPUT_SHIFT_MODIFIER;
	if (mods&GLFW_MOD_ALT)
		mmap|=GINPUT_ALT_MODIFIER;
	if (mods&GLFW_MOD_CONTROL)
		mmap|=GINPUT_CTRL_MODIFIER;
	if (mods&GLFW_MOD_SUPER)
		mmap|=GINPUT_META_MODIFIER;
	if (act) {
		mButtons|=(1<<bmap);
		ginputp_mouseDown(mpos_x,mpos_y,bmap,mmap);
	}
	else {
		mButtons&=~(1<<bmap);
		ginputp_mouseUp(mpos_x,mpos_y,bmap,mmap);		
	}
	//printf("MOUSEB:%d,%d,%d\n",btn,act,mods);
}
void cb_scroll(GLFWwindow *win,double xoff, double yoff) {
	ginputp_mouseWheel(mpos_x,mpos_y,mButtons,yoff*120,mMods);
}
// ######################################################################

int main(int argc, char *argv[])
{	
	for (int i=0;i<argc;i++)
		PATH_CommandLine.push_back(std::string(argv[i]));
	PATH_AppName=szAppName;
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
	application_=s_applicationManager->getApplication();
	s_applicationManager->surfaceCreated();
	
	glfwSetWindowSizeCallback(glfw_win,cb_winsize);
	glfwSetKeyCallback(glfw_win,cb_key);
	glfwSetCharCallback(glfw_win,cb_char);
	glfwSetCursorPosCallback(glfw_win,cb_cursorpos);
	glfwSetCursorEnterCallback(glfw_win,cb_cursorenter);
	glfwSetMouseButtonCallback(glfw_win,cb_mousebtn);
	glfwSetScrollCallback(glfw_win,cb_scroll);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(glfw_win,GLFW_RAW_MOUSE_MOTION,GLFW_TRUE);
			
	glfwMakeContextCurrent(glfw_win);
    while(!glfwWindowShouldClose(glfw_win))
    {
		if (resized) {
		  s_applicationManager->surfaceChanged(defWidth,defHeight,(defWidth>defHeight)?90:0);
		}

		if (s_applicationManager->drawFrame()) {
			glfwSwapBuffers(glfw_win);
		}

		glfwPollEvents();
    }

	glfwDestroyWindow(glfw_win);
	glfwTerminate();
   return 0;
}

#include <applicationmanager.h>
#ifdef EGL
#include <EGL/egl.h>
#else
#include <GL/glfw.h>
#endif
#include <iostream>
#include "emscripten.h"
#include "html5.h"
#include <sys/stat.h>
#include <gstdio.h>
#include <glog.h>
#include <ginput-js.h>
#include <gplugin.h>
#include <dlfcn.h>
static ApplicationManager *s_applicationManager = NULL;

#ifdef EGL
EGLDisplay display;
#endif
float pixelRatio=1.0;

static void errorAbort(const char *detail)
{
	const char *type="genErr";
	EM_ASM_( { Module.showError(Pointer_stringify($0),Pointer_stringify($1)) }, type, detail);
	emscripten_force_exit(1);
}

static void errorLua(const char *detail)
{
	const char *type="luaErr";
	EM_ASM_( { Module.showError(Pointer_stringify($0),Pointer_stringify($1)) }, type, detail);
	emscripten_force_exit(1);
}

int initGL(int width, int height)
{
 //emscripten_set_canvas_size(width,height);
#ifndef EGL
 if (glfwInit() != GL_TRUE) {
  printf("glfwInit() failed\n");
  return GL_FALSE;
 }
 
                      
 if (glfwOpenWindow(width, height, 8, 8, 8, 0, 16, 8, GLFW_WINDOW) != GL_TRUE) {
    printf("glfwOpenWindow() failed\n");
    return GL_FALSE;
 }
 
/* float ratio=1.0;//emscripten_get_device_pixel_ratio();
 EM_ASM_({
   var canvas = Module['canvas'];
   canvas.width=$0;
   canvas.height=$1;
   },width*ratio,height*ratio);    */ 
#else
 EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
 EGLint major = 0, minor = 0;
 EGLBoolean ret = eglInitialize(display, &major, &minor);
 EGLint numConfigs;
 ret = eglGetConfigs(display, NULL, 0, &numConfigs);
       
  EGLint attribs[] = {
     EGL_RED_SIZE, 8,
     EGL_GREEN_SIZE, 8,
     EGL_BLUE_SIZE, 8,
     EGL_STENCIL_SIZE, 8,
     EGL_DEPTH_SIZE, 16,
     EGL_NONE
 };
 EGLConfig config;
 ret = eglChooseConfig(display, attribs, &config, 1, &numConfigs);

 EGLNativeWindowType dummyWindow;
 EGLSurface surface = eglCreateWindowSurface(display, config, dummyWindow, NULL);
                                                                                                                    
 // WebGL maps to GLES2. GLES1 is not supported.
 // The correct attributes, should create a good EGL context
 EGLint contextAttribs[] =
 {
  EGL_CONTEXT_CLIENT_VERSION, 2,
  EGL_NONE
 };
 EGLContext context = eglCreateContext(display, config, NULL, contextAttribs);
 ret = eglMakeCurrent(display, surface, surface, context);*/
#endif
 return 0;
}

extern "C" void GGStreamOpenALTick();                                                                                                    
void looptick()
{
	try {
		s_applicationManager->drawFrame();
#ifndef EGL
    glfwSwapBuffers();
#else
    eglSwapInterval(display,1);
#endif
	}
	catch(const luaException& e)
	{
	    errorLua(e.what());
	}
	catch(const std::exception& e)
	{
		errorAbort(e.what());
	}
	catch(...)
	{
		errorAbort("Generic error");
	}
}

EM_BOOL resize_callback(int eventType, const EmscriptenUiEvent *e, void *userData)
{
 int defWidth=e->windowInnerWidth;
 int defHeight=e->windowInnerHeight;	
// printf("Resize:%d,%d\n",e->windowInnerWidth,e->windowInnerHeight);
 float ratio=emscripten_get_device_pixel_ratio();
 defWidth*=ratio;
 defHeight*=ratio;
 pixelRatio=ratio;
// printf("CanvasSize: %d,%d (%f)\n",defWidth,defHeight,ratio);
 
 //emscripten_set_canvas_size(width*ratio,height*ratio);
 glfwCloseWindow();
  initGL(defWidth,defHeight);   
//glfwSetWindowSize(defWidth,defHeight); 
    s_applicationManager->surfaceChanged(defWidth,defHeight,(defWidth>defHeight)?90:0);
 return false;
}


// returns the number of utf8 code points in the buffer at s
size_t utf8len(const char *s)
{
    size_t len = 0;
    for (; *s; ++s) if ((*s & 0xC0) != 0x80) ++len;
    return len;
}

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
 const char *key=e->key;
 char kcode[2]={(char)(e->keyCode),0};
 if ((!key)||(!(*key)))
  key=kcode;
 if (!(*key))
  kcode[0]=e->which;
 int skey=0;
 if ((!strcmp(key,"Tab"))||(*key=='\t')) skey=1;
 if ((!strcmp(key,"Backspace"))||(*key=='\b')) skey=2;
 if ((!strcmp(key,"Enter"))||(*key=='\r')) skey=4;
 if ((!strcmp(key,"Escape"))||(*key=='\e')) skey=8;
 //printf("PressCode:%s %d (%d)\n",key,skey,eventType);
 if (eventType == EMSCRIPTEN_EVENT_KEYDOWN)
 {
	 if (!e->repeat)
		 ginputp_keyDown(key,e->code);
  //printf("DownCode:%s %d\n",key,skey);
  if (skey==1) ginputp_keyChar("\t");
  if (skey==2) ginputp_keyChar("\b");
  if (skey==4) ginputp_keyChar("\r");
  if (skey==8) ginputp_keyChar("\e");
  //Emulate keypress
  if( (!skey)&&(*key))
  {
	  if ((utf8len(key)==1)||((*key)<'A')||((*key)>'Z'))
		   ginputp_keyChar(key);
  }
 }
 else if (eventType == EMSCRIPTEN_EVENT_KEYUP)
 {
  //printf("UpCode:%s %d\n",key,skey);
  ginputp_keyUp(key,e->code);
 }
 else if (eventType == EMSCRIPTEN_EVENT_KEYPRESS)
 {
	 //Keypress is being deprecated and no longer works in some situations, rely on keydown instead
/*  printf("PressCode:%s %d\n",key,skey);
  if ((utf8len(key)==1)&&(!skey))
   ginputp_keyChar(key);*/
 }

 return true;
}

EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
	 int x=e->canvasX*pixelRatio;
	 int y=e->canvasY*pixelRatio;
	 int b=e->buttons;
	 int bs=0,m=0;
	 if (e->button==0)
	  bs=1;
         else if (e->button==1)
          bs=4;
         else if (e->button==2)
          bs=2;
	 b=(b&1)|((b&2)<<1)|((b&4)>>1); //Convert buttons to gideros mask
	 if (e->ctrlKey) m|=GINPUT_CTRL_MODIFIER;
	 if (e->shiftKey) m|=GINPUT_SHIFT_MODIFIER;
	 if (e->altKey) m|=GINPUT_ALT_MODIFIER;
	 if (e->metaKey) m|=GINPUT_META_MODIFIER;

	 if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN)
		 ginputp_mouseDown(x,y,bs,m);
	 else if (eventType == EMSCRIPTEN_EVENT_MOUSEUP)
		 ginputp_mouseUp(x,y,bs,m);
	 else if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE)
	 {
		 if (b)
			 ginputp_mouseMove(x,y,b,m);
		 else
			 ginputp_mouseHover(x,y,b,m);
	}

  return true;
}

EM_BOOL wheel_callback(int eventType, const EmscriptenWheelEvent *e, void *userData)
{
 double w=e->deltaY;
 if (e->deltaMode==1)
  w=w*40;
 if (e->deltaMode==2)
  w=w*120;
 int x=e->mouse.canvasX*pixelRatio;
 int y=e->mouse.canvasY*pixelRatio;
 int b=e->mouse.buttons;
 b=(b&1)|((b&2)<<1)|(b&4>>1); //Convert buttons to gideros mask
  ginputp_mouseWheel(x,y,b,-w,0);
  return true;
}

EM_BOOL touch_callback(int eventType, const EmscriptenTouchEvent *e, void *userData)
{
	int m=0;
	 if (e->ctrlKey) m|=GINPUT_CTRL_MODIFIER;
	 if (e->shiftKey) m|=GINPUT_SHIFT_MODIFIER;
	 if (e->altKey) m|=GINPUT_ALT_MODIFIER;
	 if (e->metaKey) m|=GINPUT_META_MODIFIER;
    for (int k=0;k<e->numTouches;k++) {
     if (!e->touches[k].isChanged) continue;
	 int x=e->touches[k].canvasX*pixelRatio;
	 int y=e->touches[k].canvasY*pixelRatio;
	 int i=e->touches[k].identifier;
	 if (eventType == EMSCRIPTEN_EVENT_TOUCHSTART)
		 ginputp_touchBegin(x,y,i,m);
	 else if (eventType == EMSCRIPTEN_EVENT_TOUCHEND)
	 {
	         EM_ASM( Module.checkALMuted(); );	         
		 ginputp_touchEnd(x,y,i,m);
         }
	 else if (eventType == EMSCRIPTEN_EVENT_TOUCHMOVE)
		 ginputp_touchMove(x,y,i,m);
	 else if (eventType == EMSCRIPTEN_EVENT_TOUCHCANCEL)
		 ginputp_touchCancel(x,y,i,m);
  }
  return true;
}


extern "C" int main_registerPlugin(const char *pname);
extern "C" void* g_pluginMain_JSNative(lua_State* L, int type);

extern const char *codeKey_;
const char *currentUrl=NULL;
int main() {
  EM_ASM(Module.setStatus("Initializing"));
  try {
          
char *url=(char *) EM_ASM_INT_V({
 return allocate(intArrayFromString(location.href), 'i8', ALLOC_STACK);
});
 char *surl=(char *)malloc(strlen(url)+2);
 *surl='s';
 strcpy(surl+1,url);
 currentUrl=surl;

 const char *hostname=codeKey_+32;
 bool allowed=(strlen(hostname)==0);
 if (!allowed)
 {
  char *s1=strstr(url,"://");
  if (!s1) 
   s1=url;
  else 
   s1=s1+3;
  char *s2=strchr(s1,'/');
  if (s2) *s2=0;
  //printf("Hostname %s, required %s\n",s1,hostname);
  allowed|=!strncmp(s1,hostname,strlen(hostname));
  if (s2) *s2='/';
 }
 /*
 allowed|=!strncmp(url,"http://hieroglyphe.net/",23);
 allowed|=!strncmp(url,"http://www.geopisteur.com/",26);
 allowed|=!strncmp(url,"http://apps.giderosmobile.com/",30); 
 allowed|=!strncmp(url,"http://www.totebogames.com/",27); 
 allowed|=!strncmp(url,"http://www.miniclip.com/",24); 
 */
 if (!allowed)
 {
  printf("Sorry: location %s not allowed\n",url);
  return -1;
 }
  url=strstr(url,"://")+2;
  char *lurl=strchr(url,'?');
  if (lurl) *lurl=0;
  lurl=url+strlen(url)-1;
  if ((*lurl)=='/') *lurl=0;
  
  //PLUGINS Init
  g_registerPlugin(g_pluginMain_JSNative);
  EM_ASM(Module.registerPlugins());

  int defWidth=EM_ASM_INT_V({ return window.innerWidth; });
   int defHeight=EM_ASM_INT_V({ return window.innerHeight; });
   int fullScreen;
 float ratio=emscripten_get_device_pixel_ratio();
 defWidth*=ratio;
 defHeight*=ratio;
 pixelRatio=ratio;
// printf("CanvasSize: %d,%d (%f)\n",defWidth,defHeight,ratio);
//   emscripten_get_canvas_size(&defWidth,&defHeight,&fullScreen);
    initGL(defWidth,defHeight);    
//    glog_setLevel(0);
    bool hasGApp=EM_ASM_INT_V({ return Module.hasGApp; });
    s_applicationManager=new ApplicationManager(!hasGApp,hasGApp?"main.gapp":"",url);
    s_applicationManager->surfaceCreated();

    EMSCRIPTEN_RESULT ret;
    bool capture=false;
    ret = emscripten_set_resize_callback(0, 0, true, resize_callback);
    ret = emscripten_set_mousedown_callback(0, 0, capture, mouse_callback);
    ret = emscripten_set_mouseup_callback(0, 0, capture, mouse_callback);
    ret = emscripten_set_mousemove_callback(0, 0, capture, mouse_callback);
    ret = emscripten_set_wheel_callback(0, 0, capture, wheel_callback);
    ret = emscripten_set_touchstart_callback(0, 0, capture, touch_callback);
    ret = emscripten_set_touchend_callback(0, 0, capture, touch_callback);
    ret = emscripten_set_touchmove_callback(0, 0, capture, touch_callback);
    ret = emscripten_set_touchcancel_callback(0, 0, capture, touch_callback);
    ret = emscripten_set_keydown_callback(0, 0, true, key_callback);
    ret = emscripten_set_keyup_callback(0, 0, true, key_callback);
    ret = emscripten_set_keypress_callback(0, 0, true, key_callback);
   //printf("URL:%s\n",url);

    s_applicationManager->surfaceChanged(defWidth,defHeight,(defWidth>defHeight)?90:0);
    emscripten_set_main_loop(looptick, 0, 1);
    main_registerPlugin(NULL);
  }
  catch(const luaException& e)
  {
      errorLua(e.what());
  }
  catch(const std::exception& e)
  {
      errorAbort(e.what());
  }
  catch(...)
  {
      errorAbort("Generic error");
  }
}

int main_registerPlugin(const char *pname)
{
 if (!pname)
  return 0;
 void *hndl = dlopen (NULL, RTLD_LAZY);
 if (!hndl) { fprintf(stderr, "dlopen failed: %s\n", dlerror()); 
           exit (EXIT_FAILURE); 
 };
 void *(*func)(lua_State *,
       int)=(void *(*)(lua_State *,
             int))dlsym(hndl,pname);
 int ret=0;
 if (func)
  ret=g_registerPlugin(func);
 else
  fprintf(stderr,"Symbol %s not found\n",pname);
 //dlclose(hndl); //XXX check this
  return ret;
}

void flushDrive(int drive)
{
 if (drive==1)
 {
  printf("Sync FS\n");
  EM_ASM({
   FS.syncfs(function (err) {
           });
  });
 }
}

extern "C" void JSPlayer_play(const char *project)
{
	s_applicationManager->openProject(project);
}

extern "C" void JSPlayer_stop()
{
	s_applicationManager->stop();
}

extern "C" void JSPlayer_writeFile(const char *project, const char *path,const char *data,int datasize)
{
	char tmp[PATH_MAX];
	char *p = NULL;
	size_t len;
	snprintf(tmp, sizeof(tmp), "gideros/%s/resource/%s", project,path);
	len = strlen(tmp);
	if (tmp[len - 1] == '/')
		tmp[len - 1] = 0;
	for (p = tmp + 1; *p; p++)
		if (*p == '/') {
			*p = 0;
			mkdir(tmp, S_IRWXU);
			*p = '/';
		}
	FILE* fos = fopen(tmp, "wb");
	fwrite(data,datasize, 1, fos);
	fclose(fos);
	glog_v("Wrote file '%s' (%d)\n",tmp,datasize);
}

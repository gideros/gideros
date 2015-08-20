#include <applicationmanager.h>
#ifdef EGL
#include <EGL/egl.h>
#else
#include <GL/glfw.h>
#endif
#include <iostream>
#include "emscripten.h"
#include "html5.h"
#include <glog.h>

static ApplicationManager *s_applicationManager = NULL;

#ifdef EGL
EGLDisplay display;
#endif

int initGL(int width, int height)
{
 //emscripten_set_canvas_size(width,height);
#ifndef EGL
 if (glfwInit() != GL_TRUE) {
  printf("glfwInit() failed\n");
  return GL_FALSE;
 }
                      
 if (glfwOpenWindow(width, height, 8, 8, 8, 8, 16, 0, GLFW_WINDOW) != GL_TRUE) {
    printf("glfwOpenWindow() failed\n");
    return GL_FALSE;
 }
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
    s_applicationManager->drawFrame();
#ifndef EGL
    glfwSwapBuffers();
#else
    eglSwapInterval(display,1);
#endif
}

extern "C" void g_setFps(int fps)
{
}



EM_BOOL mouse_callback(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
	 int x=e->canvasX;
	 int y=e->canvasY;
	 int b=e->buttons;
	 b=(b&1)|((b&2)<<1)|(b&4>>1); //Convert buttons to gideros mask
	 if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN)
		 ginputp_mouseDown(x,y,b);
	 else if (eventType == EMSCRIPTEN_EVENT_MOUSEUP)
		 ginputp_mouseUp(x,y,b);
	 else if (eventType == EMSCRIPTEN_EVENT_MOUSEMOVE)
	 {
		 if (b)
			 ginputp_mouseMove(x,y,b);
		 else
			 ginputp_mouseHover(x,y,b);
	}

  return 0;
}

EM_BOOL wheel_callback(int eventType, const EmscriptenWheelEvent *e, void *userData)
{
 printf("Wheel:%d Mode:%d\n",e->deltaY,e->deltaMode);
 //ginputp_mouseWheel();
  return 0;
}


int main() {
   int defWidth=320;
   int defHeight=480;
    initGL(defWidth,defHeight);    
//    glog_setLevel(0);
    s_applicationManager=new ApplicationManager(false,"main.gapp");
    s_applicationManager->surfaceCreated();

    EMSCRIPTEN_RESULT ret;
    //ret = emscripten_set_click_callback(0, 0, 1, mouse_callback);
    ret = emscripten_set_mousedown_callback(0, 0, 1, mouse_callback);
    ret = emscripten_set_mouseup_callback(0, 0, 1, mouse_callback);
    //ret = emscripten_set_dblclick_callback(0, 0, 1, mouse_callback);
    ret = emscripten_set_mousemove_callback(0, 0, 1, mouse_callback);
    ret = emscripten_set_wheel_callback(0, 0, 1, wheel_callback);

    s_applicationManager->surfaceChanged(defWidth,defHeight,0);
    emscripten_set_main_loop(looptick, 0, 1);
}


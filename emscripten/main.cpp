#include <applicationmanager.h>
#ifdef EGL
#include <EGL/egl.h>
#else
#include <GL/glfw.h>
#endif
#include <iostream>
#include "emscripten.h"
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

int main() {
    initGL(240,320);    
    glog_setLevel(0);
    s_applicationManager=new ApplicationManager(false);
    s_applicationManager->surfaceCreated();
    s_applicationManager->surfaceChanged(240,320,0);
    emscripten_set_main_loop(looptick, 0, 1);
        
}


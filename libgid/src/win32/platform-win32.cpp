#include <stdio.h>
#include <windows.h>
#include <vector>
#include <string>
#include <stdlib.h>

#include "luaapplication.h"
#include <application.h>

extern HWND hwndcopy;
extern char commandLine[];
// extern int dxChrome,dyChrome;
extern LuaApplication *application_;

static RECT winRect;
static bool isFullScreen=false;

void GetDesktopResolution(int& horizontal, int& vertical)
{
  RECT desktop;
  // Get a handle to the desktop window
  const HWND hDesktop = GetDesktopWindow();
  // Get the size of screen to the variable desktop
  GetClientRect(hDesktop, &desktop);
  // The top left corner will have coordinates (0,0)
  // and the bottom right corner will have coordinates
  // (horizontal, vertical)
  horizontal = desktop.right;
  vertical = desktop.bottom;
}

std::vector<std::string> getDeviceInfo()
{
  std::vector<std::string> result;

  result.push_back("Win32");
  
  return result;
}

void openUrl(const char* url)
{
  ShellExecute(hwndcopy,NULL,url,NULL,NULL,SW_SHOWNORMAL);
}

bool canOpenUrl(const char *url)
{
  return true;
}

std::string getLocale()
{
  TCHAR szBuff1[10], szBuff2[10]; 

  LCID lcid = GetUserDefaultLCID(); 

  GetLocaleInfo(lcid, LOCALE_SISO639LANGNAME, szBuff1, 10); 
  GetLocaleInfo(lcid, LOCALE_SISO3166CTRYNAME, szBuff2, 10); 

  strcat(szBuff1,"_");
  strcat(szBuff1,szBuff2);

  return szBuff1;
}

std::string getLanguage()
{
  TCHAR szBuff[10]; 
  LCID lcid = GetUserDefaultLCID(); 
  GetLocaleInfo(lcid, LOCALE_SISO639LANGNAME, szBuff, 10); 
  return szBuff;
}

void setWindowSize(int width, int height)
{
  printf("setWindowSize: %d x %d. hwndcopy=%p\n",width,height,hwndcopy);

  Orientation app_orient=application_->orientation();

  if (app_orient==ePortrait || app_orient==ePortraitUpsideDown){
    RECT rect;
    rect.left=0;
    rect.top=0;
    rect.right=width;
    rect.bottom=height;

    AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);

    SetWindowPos(hwndcopy,HWND_TOP,0,0,rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE);
    printf("SetWindowPos: %d %d\n",rect.right-rect.left, rect.bottom-rect.top);
  }
  else {
    RECT rect;
    rect.left=0;
    rect.top=0;
    rect.right=height;
    rect.bottom=width;

    AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);

    SetWindowPos(hwndcopy,HWND_TOP,0,0,rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE);
    printf("SetWindowPos: %d %d\n",rect.right-rect.left, rect.bottom-rect.top);
  }

  application_->setHardwareOrientation(app_orient);   // previously eFixed
  application_->getApplication()->setDeviceOrientation(app_orient);
}

void setFullScreen(bool fullScreen)
{

  if (fullScreen==isFullScreen) return;

  if (fullScreen){
    GetWindowRect(hwndcopy,&winRect);    // store the current windows rectangle

    int horizontal,vertical;
    GetDesktopResolution(horizontal,vertical);
    SetWindowPos(hwndcopy,HWND_TOPMOST,0,0,horizontal,vertical,0);
  }
  else {
    SetWindowPos(hwndcopy,HWND_NOTOPMOST,0,0,winRect.right,winRect.bottom,0);
  }
  isFullScreen=fullScreen;
}

void vibrate(int ms)
{
}

void setKeepAwake(bool awake)
{
}

bool setKeyboardVisibility(bool visible){
	return false;
}

static int s_fps = 60;

extern "C" {

int g_getFps()
{
  return s_fps;
}

void g_setFps(int fps)
{
  s_fps = fps;
}

}

void g_exit()
{
  exit(0);
}

bool g_checkStringProperty(bool isSet, const char* what){
    return false;
}

void g_setProperty(const char* what, const char* arg){

}

const char* g_getProperty(const char* what, const char* arg)
{
  if (strcmp(what,"commandLine")==0)
    return commandLine;
}

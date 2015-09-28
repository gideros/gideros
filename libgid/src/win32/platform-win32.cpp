#include <windows.h>
#include <vector>
#include <string>

#include <stdlib.h>

extern HWND hwndcopy;
extern char commandLine[];
extern int dxChrome,dyChrome;

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
  return "placeholder";
}

std::string getLanguage()
{
  return "placeholder";
}

void setWindowSize(int width, int height)
{
  SetWindowPos(hwndcopy,HWND_TOP,0,0,width+dxChrome,height+dyChrome,SWP_NOMOVE);
}

void setFullScreen(bool fullScreen)
{

  if (fullScreen==isFullScreen) return;

  if (fullScreen){
    GetWindowRect(hwndcopy,&winRect);

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

void g_setProperty(const char* what, const char* arg)
{

}

const char* g_getProperty(const char* what)
{
  if (strcmp(what,"commandLine")==0)
    return commandLine;
}

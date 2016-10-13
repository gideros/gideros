#include <vector>
#include <string>

#include <stdlib.h>

void GetDesktopResolution(int& horizontal, int& vertical)
{
}

std::vector<std::string> getDeviceInfo()
{
  std::vector<std::string> result;

  result.push_back("pi");
  
  return result;
}

void openUrl(const char* url)
{
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
}

void setFullScreen(bool fullScreen)
{
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
   return NULL;
}

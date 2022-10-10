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

std::string getAppId(){
	return "";
}

void getSafeDisplayArea(int &x,int &y,int &w,int &h)
{
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

bool setTextInput(int type,const char *buffer,int selstart,int selend,const char *label,const char *actionLabel, const char *hintText)
{
	return false;
}

int setClipboard(std::string data,std::string mimeType, int luaFunc) {
	return -1;
}

int getClipboard(std::string &data,std::string &mimeType, int luaFunc) {
	return -1;
}

int getKeyboardModifiers() {
   return 0;
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

std::vector<gapplication_Variant> g_getsetProperty(bool set, const char* what, std::vector<gapplication_Variant> &args)
{
	std::vector<gapplication_Variant> rets;
	gapplication_Variant r;
/*	if (!set) {
		if (!strcmp(what,"currentUrl"))
		{
			r.type=gapplication_Variant::STRING;
			r.s=currentUrl;
			rets.push_back(r);
		}
	}*/
	return rets;
}

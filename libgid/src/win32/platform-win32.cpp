#include <windows.h>
#include <vector>
#include <string>

#include <stdlib.h>

extern HWND hwndcopy;

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

}

std::string getLanguage()
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

void g_setProperty(const char* what, const char* arg){

}

const char* g_getProperty(const char* what){

}

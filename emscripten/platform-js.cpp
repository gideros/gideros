#include <platform.h>
#include "emscripten.h"
#include "html5.h"

std::vector<std::string> getDeviceInfo()
{
  std::vector<std::string> result;
        
  result.push_back("Web");
                                
 return result;
}
                                        
void setKeepAwake(bool awake)
{
}

bool setKeyboardVisibility(bool visible){
	return false;
}

void vibrate(int ms)
{
	EM_ASM_({
		window.navigator.vibrate($0);
	},ms);
}

void setWindowSize(int width, int height){
	//TODO
}

void setFullScreen(bool fullScreen){
	emscripten_request_fullscreen("canvas",true);
}

std::string getDeviceName(){
	return ""; //TODO
}

std::string getLocale()
{
	return ""; //TODO
}

std::string getLanguage()
{
	return "en"; //TODO
}

void openUrl(const char* url)
{
	EM_ASM_({
		window.open(Pointer_stringify($0));
	},url);
}

bool canOpenUrl(const char *url)
{
    return true;
}

bool g_checkStringProperty(bool isSet, const char* what)
{
 return false;
}

extern const char *currentUrl;
const char* g_getProperty(const char* what, const char* arg)
{
 if (!strcmp(what,"currentUrl"))
  return currentUrl;
 return NULL;
}

void g_setProperty(const char* what, const char* arg)
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

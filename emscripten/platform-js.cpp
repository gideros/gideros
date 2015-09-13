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

void vibrate(int ms)
{
	EM_ASM_({
		window.navigator.vibrate(ms);
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


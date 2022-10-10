#include <platform.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>

std::vector<std::string> getDeviceInfo()
{
  std::vector<std::string> result;
        
  result.push_back("Web");
	char *userAgentC=(char *) EM_ASM_INT_V({
	 return allocate(intArrayFromString(navigator.userAgent), 'i8', ALLOC_STACK);
	});
  std::string userAgent=userAgentC;
  result.push_back(userAgent);
                                
 return result;
}

std::string getAppId(){
	return "";
}

void getSafeDisplayArea(int &x,int &y,int &w,int &h)
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
	if ((mimeType=="")||(mimeType=="text/plain")) {
		EM_ASM_({
			if (window.navigator.clipboard && window.navigator.clipboard.writeText) {
				window.navigator.clipboard.writeText(UTF8ToString($1)).then(
					function() { Module._gapplication_clipboardCallback($0,1,0,0) }
				).catch(
					function() { Module._gapplication_clipboardCallback($0,-1,0,0) }
				);
			}
			else
				Module._gapplication_clipboardCallback($0,-1,"","");
		},luaFunc,data.c_str());
		return 0;
	}
	return -1;
}

int getClipboard(std::string &data,std::string &mimeType, int luaFunc) {
	if ((mimeType=="")||(mimeType=="text/plain")) {
		EM_ASM_({
			if (window.navigator.clipboard && window.navigator.clipboard.readText) {
				window.navigator.clipboard.readText().then(function (clipText) {
						Module._gapplication_clipboardCallback($0,1,
								allocate(intArrayFromString(clipText), 'i8', ALLOC_STACK),
								allocate(intArrayFromString("text/plain"), 'i8', ALLOC_STACK))
					}
				).catch(
					err => Module._gapplication_clipboardCallback($0,-1,0,0)
				);
			}
			else
				Module._gapplication_clipboardCallback($0,-1,0,0);
		},luaFunc);
		return 0;
	}
	return -1;
}

extern int s_KeyboardModifiers;
int getKeyboardModifiers() {
	return s_KeyboardModifiers;
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
	char *lang=(char *) EM_ASM_INT_V({
	 return allocate(intArrayFromString(Module.gplatformLanguage()), 'i8', ALLOC_STACK);
	});

	return lang;
}

void openUrl(const char* url)
{
	EM_ASM_({
		window.open(UTF8ToString($0));
	},url);
}

bool canOpenUrl(const char *url)
{
    return true;
}

extern const char *currentUrl;
std::vector<gapplication_Variant> g_getsetProperty(bool set, const char* what, std::vector<gapplication_Variant> &args)
{
	std::vector<gapplication_Variant> rets;
	gapplication_Variant r;
	if (!set) {
		if (!strcmp(what,"currentUrl"))
		{
			r.type=gapplication_Variant::STRING;
			r.s=currentUrl;
			rets.push_back(r);
		}
	}
	return rets;
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
